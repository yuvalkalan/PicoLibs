#include "E01C2G4M27D.h"
#include "E01C2G4M27D_REG.h"
#include "hardware/timer.h"
#include <string.h>

E01C2G4M27D::E01C2G4M27D(spi_inst_t *spi_port, uint pin_ce, uint pin_csn, uint pin_sck, uint pin_mosi, uint pin_miso, uint pin_irq)
    : spi(spi_port),
      pin_ce(pin_ce),
      pin_csn(pin_csn),
      pin_sck(pin_sck),
      pin_mosi(pin_mosi),
      pin_miso(pin_miso),
      pin_irq(pin_irq),
      payload_size(32)
{
}

void E01C2G4M27D::begin()
{
    // Initialize hardware SPI at 4MHz
    spi_init(spi, 4000000);
    spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(pin_miso, GPIO_FUNC_SPI);
    gpio_set_function(pin_sck, GPIO_FUNC_SPI);
    gpio_set_function(pin_mosi, GPIO_FUNC_SPI);

    // CSN (Chip Select Not) setup
    gpio_init(pin_csn);
    gpio_set_dir(pin_csn, GPIO_OUT);
    csnHigh();

    // CE (Chip Enable) setup
    gpio_init(pin_ce);
    gpio_set_dir(pin_ce, GPIO_OUT);
    ceLow();

    sleep_ms(5); // Wait for module to settle on boot

    // Configuration settings
    writeRegister(REG_CONFIG, 0x0C);              // 16-bit CRC, PWR_UP = 0
    writeRegister(REG_EN_AA, 0x3F);               // Auto-Ack on all pipes
    writeRegister(REG_EN_RXADDR, 0x03);           // Enable Data Pipes 0 & 1
    writeRegister(REG_SETUP_AW, 0x03);            // 5-byte address width
    writeRegister(REG_SETUP_RETR, (5 << 4) | 15); // Auto retransmit: 1500us delay, 15 retries

    // Power up in RX mode
    writeRegister(REG_CONFIG, readRegister(REG_CONFIG) | 0x02 | 0x01);
    sleep_ms(2);
}

void E01C2G4M27D::setChannel(uint8_t channel)
{
    writeRegister(REG_RF_CH, channel & 0x7F);
}

void E01C2G4M27D::setPALevel(uint8_t level)
{
    uint8_t setup = readRegister(REG_RF_SETUP) & ~0x06;
    if (level > 3)
        level = 3;
    setup |= (level << 1);
    writeRegister(REG_RF_SETUP, setup);
}

void E01C2G4M27D::setDataRate(uint8_t rate)
{
    uint8_t setup = readRegister(REG_RF_SETUP) & ~0x28; // Clear bits 5 (250k) and 3 (2M)
    if (rate == 1)
        setup |= (1 << 3); // 2Mbps
    else if (rate == 2)
        setup |= (1 << 5); // 250kbps
    writeRegister(REG_RF_SETUP, setup);
}

void E01C2G4M27D::openWritingPipe(const uint8_t *address)
{
    writeRegisterMulti(REG_TX_ADDR, address, 5);
    writeRegisterMulti(REG_RX_ADDR_P0, address, 5); // Required for auto-ack packets
    writeRegister(REG_RX_PW_P0, payload_size);
}

void E01C2G4M27D::openReadingPipe(uint8_t number, const uint8_t *address)
{
    if (number == 0)
    {
        writeRegisterMulti(REG_RX_ADDR_P0, address, 5);
        writeRegister(REG_RX_PW_P0, payload_size);
    }
    else if (number == 1)
    {
        writeRegisterMulti(REG_RX_ADDR_P1, address, 5);
        writeRegister(REG_RX_PW_P1, payload_size);
    }

    uint8_t en_rxaddr = readRegister(REG_EN_RXADDR);
    en_rxaddr |= (1 << number);
    writeRegister(REG_EN_RXADDR, en_rxaddr);
}

void E01C2G4M27D::startListening()
{
    writeRegister(REG_CONFIG, readRegister(REG_CONFIG) | 0x01); // SET PRIM_RX
    writeRegister(REG_STATUS, 0x70);                            // Clear interrupts
    ceHigh();
    sleep_us(130); // Allow PLL to settle
}

void E01C2G4M27D::stopListening()
{
    ceLow();
    sleep_us(130);
    flushTx();
    flushRx();
    writeRegister(REG_CONFIG, readRegister(REG_CONFIG) & ~0x01); // CLEAR PRIM_RX
}

bool E01C2G4M27D::write(const void *buf, uint8_t len)
{
    stopListening();

    csnLow();
    spiTransfer(CMD_W_TX_PAYLOAD);
    spi_write_blocking(spi, (const uint8_t *)buf, len);
    // Pad remaining payload space with zeros
    if (payload_size > len)
    {
        uint8_t padding[32] = {0};
        spi_write_blocking(spi, padding, payload_size - len);
    }
    csnHigh();

    // Pulse CE to transmit
    ceHigh();
    sleep_us(15);
    ceLow();

    // Wait for TX Data Sent (TX_DS) or Max Retries (MAX_RT) flag
    uint8_t status = 0;
    uint32_t timeout = to_ms_since_boot(get_absolute_time()) + 15;
    while (true)
    {
        status = getStatus();
        if (status & 0x30)
            break; // 0x20 = TX_DS, 0x10 = MAX_RT
        if (to_ms_since_boot(get_absolute_time()) > timeout)
            break;
        sleep_us(50);
    }

    writeRegister(REG_STATUS, 0x30); // Clear interrupt flags

    if (status & 0x10)
    {
        flushTx();
        return false; // Retries exhausted
    }
    return (status & 0x20) != 0; // Success if Data Sent is high
}

bool E01C2G4M27D::available()
{
    uint8_t fifo_status = readRegister(REG_FIFO_STATUS);
    if (!(fifo_status & 0x01))
    {                                    // 0x01 is RX_EMPTY
        writeRegister(REG_STATUS, 0x40); // Clear RX_DR interrupt
        return true;
    }
    return false;
}

uint8_t E01C2G4M27D::getObserveTx()
{
    return readRegister(REG_OBSERVE_TX);
}

bool E01C2G4M27D::getRPD()
{
    // Returns true if the received signal was stronger than -64dBm
    return (readRegister(REG_RPD) & 0x01) != 0;
}

void E01C2G4M27D::read(void *buf, uint8_t len)
{
    csnLow();
    spiTransfer(CMD_R_RX_PAYLOAD);
    spi_read_blocking(spi, 0xFF, (uint8_t *)buf, len);

    // Clear out remaining payload bytes from the hardware RX FIFO
    if (payload_size > len)
    {
        uint8_t dummy[32];
        spi_read_blocking(spi, 0xFF, dummy, payload_size - len);
    }
    csnHigh();
}

// Low-Level Helper Implementations

uint8_t E01C2G4M27D::spiTransfer(uint8_t data)
{
    uint8_t rx = 0;
    spi_write_read_blocking(spi, &data, &rx, 1);
    return rx;
}

void E01C2G4M27D::csnLow() { gpio_put(pin_csn, 0); }
void E01C2G4M27D::csnHigh() { gpio_put(pin_csn, 1); }
void E01C2G4M27D::ceLow() { gpio_put(pin_ce, 0); }
void E01C2G4M27D::ceHigh() { gpio_put(pin_ce, 1); }

uint8_t E01C2G4M27D::readRegister(uint8_t reg)
{
    csnLow();
    spiTransfer(CMD_R_REGISTER | (reg & 0x1F));
    uint8_t val = spiTransfer(0xFF);
    csnHigh();
    return val;
}

void E01C2G4M27D::writeRegister(uint8_t reg, uint8_t value)
{
    csnLow();
    spiTransfer(CMD_W_REGISTER | (reg & 0x1F));
    spiTransfer(value);
    csnHigh();
}

void E01C2G4M27D::readRegisterMulti(uint8_t reg, uint8_t *buf, uint8_t len)
{
    csnLow();
    spiTransfer(CMD_R_REGISTER | (reg & 0x1F));
    spi_read_blocking(spi, 0xFF, buf, len);
    csnHigh();
}

void E01C2G4M27D::writeRegisterMulti(uint8_t reg, const uint8_t *buf, uint8_t len)
{
    csnLow();
    spiTransfer(CMD_W_REGISTER | (reg & 0x1F));
    spi_write_blocking(spi, buf, len);
    csnHigh();
}

uint8_t E01C2G4M27D::getStatus()
{
    csnLow();
    uint8_t status = spiTransfer(0xFF); // Send NOP
    csnHigh();
    return status;
}

void E01C2G4M27D::flushTx()
{
    csnLow();
    spiTransfer(CMD_FLUSH_TX);
    csnHigh();
}
void E01C2G4M27D::flushRx()
{
    csnLow();
    spiTransfer(CMD_FLUSH_RX);
    csnHigh();
}