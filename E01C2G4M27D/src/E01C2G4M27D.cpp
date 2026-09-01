#include "E01C2G4M27D.h"

E01C2G4M27D::E01C2G4M27D(spi_inst_t *spi, uint ce, uint csn, uint sck, uint mosi, uint miso, uint irq)
    : HighSPI(spi, miso, csn, sck, mosi, SPI_SPEED), m_ce(ce), m_irq(irq), m_payload_size(32)
{
}

void E01C2G4M27D::begin()
{
    // Initialize hardware SPI
    init_spi();
    spi_set_format(m_spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    // CE setup
    gpio_init(m_ce);
    gpio_set_dir(m_ce, GPIO_OUT);
    ceLow();

    // TODO: add irq here

    sleep_ms(5); // Wait for module to settle on boot

    // Configuration settings
    writeRegister(E01C2G4M27D_REG_CONFIG, 0x0C);                                             // 16-bit CRC, PWR_UP = 0
    writeRegister(E01C2G4M27D_REG_EN_AA, 0x3F);                                              // Auto-Ack on all pipes
    writeRegister(E01C2G4M27D_REG_EN_RXADDR, 0x03);                                          // Enable Data Pipes 0 & 1
    writeRegister(E01C2G4M27D_REG_SETUP_AW, 0x03);                                           // 5-byte address width
    writeRegister(E01C2G4M27D_REG_SETUP_RETR, (RETRANSMIT_DELAY << 4) | RETRANSMIT_RETRIES); // configure Auto retransmit

    // Power up in RX mode
    writeRegister(E01C2G4M27D_REG_CONFIG, readRegister(E01C2G4M27D_REG_CONFIG) | 0x02 | 0x01);
    sleep_ms(2);
}

void E01C2G4M27D::set_channel(uint8_t channel)
{
    writeRegister(E01C2G4M27D_REG_RF_CH, channel & 0x7F);
}

void E01C2G4M27D::set_PA_level(uint8_t level)
{
    uint8_t setup = readRegister(E01C2G4M27D_REG_RF_SETUP) & ~0x06;
    if (level > 3)
        level = 3;
    setup |= (level << 1);
    writeRegister(E01C2G4M27D_REG_RF_SETUP, setup);
}

void E01C2G4M27D::set_data_rate(uint8_t rate)
{
    uint8_t setup = readRegister(E01C2G4M27D_REG_RF_SETUP) & ~0x28; // Clear bits 5 (250k) and 3 (2M)
    if (rate == 1)
        setup |= (1 << 3); // 2Mbps
    else if (rate == 2)
        setup |= (1 << 5); // 250kbps
    writeRegister(E01C2G4M27D_REG_RF_SETUP, setup);
}

void E01C2G4M27D::open_writing_pipe(const uint8_t *address)
{
    writeRegisterMulti(E01C2G4M27D_REG_TX_ADDR, address, 5);
    writeRegisterMulti(E01C2G4M27D_REG_RX_ADDR_P0, address, 5); // Required for auto-ack packets
    writeRegister(E01C2G4M27D_REG_RX_PW_P0, m_payload_size);
}

void E01C2G4M27D::open_reading_pipe(uint8_t number, const uint8_t *address)
{
    if (number == 0)
    {
        writeRegisterMulti(E01C2G4M27D_REG_RX_ADDR_P0, address, 5);
        writeRegister(E01C2G4M27D_REG_RX_PW_P0, m_payload_size);
    }
    else if (number == 1)
    {
        writeRegisterMulti(E01C2G4M27D_REG_RX_ADDR_P1, address, 5);
        writeRegister(E01C2G4M27D_REG_RX_PW_P1, m_payload_size);
    }

    uint8_t en_rxaddr = readRegister(E01C2G4M27D_REG_EN_RXADDR);
    en_rxaddr |= (1 << number);
    writeRegister(E01C2G4M27D_REG_EN_RXADDR, en_rxaddr);
}

void E01C2G4M27D::start_listening()
{
    writeRegister(E01C2G4M27D_REG_CONFIG, readRegister(E01C2G4M27D_REG_CONFIG) | 0x01); // SET PRIM_RX
    writeRegister(E01C2G4M27D_REG_STATUS, 0x70);                                        // Clear interrupts
    ceHigh();
    sleep_us(130); // Allow PLL to settle
}

void E01C2G4M27D::stop_listening()
{
    ceLow();
    sleep_us(130);
    flush_tx();
    flush_rx();
    writeRegister(E01C2G4M27D_REG_CONFIG, readRegister(E01C2G4M27D_REG_CONFIG) & ~0x01); // CLEAR PRIM_RX
}

bool E01C2G4M27D::write(const void *buf, uint8_t len)
{
    stop_listening();

    uint8_t cmd = E01C2G4M27D_CMD_W_TX_PAYLOAD;
    select();
    spi_write_blocking(m_spi, &cmd, 1);
    spi_write_blocking(m_spi, (const uint8_t *)buf, len);
    // Pad remaining payload space with zeros
    if (m_payload_size > len)
    {
        uint8_t padding[32] = {0};
        spi_write_blocking(m_spi, padding, m_payload_size - len);
    }
    deselect();

    // Pulse CE to transmit
    ceHigh();
    sleep_us(15);
    ceLow();

    // Wait for TX Data Sent (TX_DS) or Max Retries (MAX_RT) flag
    uint8_t status = 0;
    uint32_t timeout = to_ms_since_boot(get_absolute_time()) + (RETRANSMIT_RETRIES * RETRANSMIT_DELAY_US) / 1000;
    while (true)
    {
        status = get_status();
        if (status & 0x30)
            break; // 0x20 = TX_DS, 0x10 = MAX_RT
        if (to_ms_since_boot(get_absolute_time()) > timeout)
            break;
        sleep_us(50);
    }

    writeRegister(E01C2G4M27D_REG_STATUS, 0x30); // Clear interrupt flags

    if (status & 0x10)
    {
        flush_tx();
        return false; // Retries exhausted
    }
    return (status & 0x20) != 0; // Success if Data Sent is high
}

bool E01C2G4M27D::available()
{
    uint8_t fifo_status = readRegister(E01C2G4M27D_REG_FIFO_STATUS);
    if (!(fifo_status & 0x01))
    {                                                // 0x01 is RX_EMPTY
        writeRegister(E01C2G4M27D_REG_STATUS, 0x40); // Clear RX_DR interrupt
        return true;
    }
    return false;
}

uint8_t E01C2G4M27D::get_observe_tx()
{
    return readRegister(E01C2G4M27D_REG_OBSERVE_TX);
}

bool E01C2G4M27D::get_RPD()
{
    // Returns true if the received signal was stronger than -64dBm
    return (readRegister(E01C2G4M27D_REG_RPD) & 0x01) != 0;
}

void E01C2G4M27D::read(void *buf, uint8_t len)
{

    uint8_t cmd = E01C2G4M27D_CMD_R_RX_PAYLOAD;
    select();
    spi_write_blocking(m_spi, &cmd, 1);
    spi_read_blocking(m_spi, 0xFF, (uint8_t *)buf, len);
    // Clear out remaining payload bytes from the hardware RX FIFO
    if (m_payload_size > len)
    {
        uint8_t dummy[32];
        spi_read_blocking(m_spi, 0xFF, dummy, m_payload_size - len);
    }
    deselect();
}
