#include "CC1101.h"

static int8_t patable_index_power[CC1101_PATABLE_SIZE] = {-30, -20, -15, -10, 0, 5, 7, 10};
static uint8_t patable_power_315[CC1101_PATABLE_SIZE] = {0x17, 0x1D, 0x26, 0x69, 0x51, 0x86, 0xCC, 0xC3};
static uint8_t patable_power_433[CC1101_PATABLE_SIZE] = {0x6C, 0x1C, 0x06, 0x3A, 0x51, 0x85, 0xC8, 0xC0};
static uint8_t patable_power_868[CC1101_PATABLE_SIZE] = {0x03, 0x17, 0x1D, 0x26, 0x50, 0x86, 0xCD, 0xC0};
static uint8_t patable_power_915[CC1101_PATABLE_SIZE] = {0x0B, 0x1B, 0x6D, 0x67, 0x50, 0x85, 0xC9, 0xC1};

// byte operators macros
#define bitRead(value, bit) (((value) >> (bit)) & 0x01) // bitRead function from arduino

void fifo_erase(uint8_t *buffer)
{
    memset(buffer, 0, CC1101_FIFOBUFFER); // erased the RX_fifo array content to "0"
}

int8_t rssi_convert(uint8_t raw_rssi)
{
    // 1. Cast the raw byte to a signed 8-bit integer so values >= 128 become negative
    int8_t signed_raw = (int8_t)raw_rssi;

    // 2. Do the math in a 16-bit integer so -138 doesn't instantly underflow
    int16_t calculated_rssi = (signed_raw / 2) - CC1101_RSSI_OFFSET_868MHZ;

    // 3. Clamp the value at -128 to prevent it from wrapping around to a positive number
    if (calculated_rssi < -128)
    {
        return -128; // Absolute noise floor
    }

    // 4. Safely return it as an 8-bit integer
    return (int8_t)calculated_rssi;
}

uint8_t lqi_convert(uint8_t lqi)
{
    return (lqi & 0x7F);
}
uint8_t check_crc(uint8_t lqi)
{
    return (lqi & 0x80);
}

void CC1101::strobe(uint8_t cmd)
{
    cc1101_select();
    spi_write_blocking(CC1101_SPI_PORT, &cmd, 1);
    cc1101_deselect();
}
void CC1101::write_single_byte(uint8_t address, uint8_t data)
{
    cc1101_select();
    uint8_t address_byte = address | CC1101_WRITE_SINGLE_BYTE;
    spi_write_blocking(CC1101_SPI_PORT, &address_byte, 1);
    spi_write_blocking(CC1101_SPI_PORT, &data, 1);
    cc1101_deselect();
}
uint8_t CC1101::read_single_byte(uint8_t address)
{
    uint8_t data;
    cc1101_select();
    uint8_t address_byte = address | CC1101_READ_SINGLE_BYTE;
    spi_write_blocking(CC1101_SPI_PORT, &address_byte, 1);
    spi_read_blocking(CC1101_SPI_PORT, 0x00, &data, 1);
    cc1101_deselect();
    return data;
}
void CC1101::write_burst(uint8_t reg_addr, uint8_t *buffer, size_t size)
{
    uint8_t addr = reg_addr | CC1101_WRITE_BURST; // Enable burst transfer
    cc1101_select();
    spi_write_blocking(CC1101_SPI_PORT, &addr, 1);
    spi_write_blocking(CC1101_SPI_PORT, buffer, size);
    cc1101_deselect();
}
void CC1101::read_burst(uint8_t *buffer, uint8_t reg_addr, uint8_t len)
{
    uint8_t addr = reg_addr | CC1101_READ_BURST;
    cc1101_select();
    spi_write_blocking(CC1101_SPI_PORT, &addr, 1);         // Send register address
    spi_read_blocking(CC1101_SPI_PORT, 0x00, buffer, len); // Read result
    cc1101_deselect();
}

void CC1101::init_pins()
{
    // Initialize SPI port
    spi_init(CC1101_SPI_PORT, CC1101_SPI_BAUDRATE);
    gpio_set_function(CC1101_PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(CC1101_PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(CC1101_PIN_SCK, GPIO_FUNC_SPI);
    // Configure Chip Select (CS) pin
    gpio_init(CC1101_PIN_CS);
    gpio_set_dir(CC1101_PIN_CS, GPIO_OUT);
    cc1101_deselect();
    // Configure GDO0 and GDO2 pins
    gpio_init(CC1101_PIN_GDO0);
    gpio_set_dir(CC1101_PIN_GDO0, GPIO_IN); // Config GDO0 as input
    gpio_init(CC1101_PIN_GDO2);
    gpio_set_dir(CC1101_PIN_GDO2, GPIO_IN); // Config GDO2 as input
}

void CC1101::set_myaddr(uint8_t addr)
{
    m_address = addr;
    write_single_byte(CC1101_ADDR, addr); // stores MyAddr in the cc1101
}
void CC1101::set_channel(uint8_t channel)
{
    m_channel = channel;
    write_single_byte(CC1101_CHANNR, channel); // stores the new channel # in the cc1101
}
void CC1101::set_ISM(uint8_t ism_freq)
{
    m_freq = ism_freq;
    uint8_t freq2, freq1, freq0;
    switch (ism_freq) // loads the RF freq which is defined in cc1101_freq_select
    {
    case CC1101_FREQ_315MHZ:
        freq2 = 0x0C;
        freq1 = 0x1D;
        freq0 = 0x89;
        write_burst(CC1101_PATABLE_BURST, patable_power_315, 8);
        break;
    case CC1101_FREQ_434MHZ:
        freq2 = 0x10;
        freq1 = 0xB0;
        freq0 = 0x71;
        write_burst(CC1101_PATABLE_BURST, patable_power_433, 8);
        break;
    case CC1101_FREQ_868MHZ:
        freq2 = 0x21;
        freq1 = 0x65;
        freq0 = 0x6A;
        write_burst(CC1101_PATABLE_BURST, patable_power_868, 8);
        break;
    case CC1101_FREQ_915MHZ:
        freq2 = 0x23;
        freq1 = 0x31;
        freq0 = 0x3B;
        write_burst(CC1101_PATABLE_BURST, patable_power_915, 8);
        break;
    default: // default is 868.3MHz
        freq2 = 0x21;
        freq1 = 0x65;
        freq0 = 0x6A;
        write_burst(CC1101_PATABLE_BURST, patable_power_868, 8); // sets up output power ramp register
        break;
    }
    write_single_byte(CC1101_FREQ2, freq2); // stores the new freq setting for defined ISM band
    write_single_byte(CC1101_FREQ1, freq1);
    write_single_byte(CC1101_FREQ0, freq0);
}
void CC1101::set_mode(uint8_t mode)
{
    m_mode = mode;
    switch (mode)
    {
    case 0x01:
        write_burst(0, cc1101_GFSK_1_2_kb, CC1101_CFG_REGISTER);
        break;
    case 0x02:
        write_burst(0, cc1101_GFSK_38_4_kb, CC1101_CFG_REGISTER);
        break;
    case 0x03:
        write_burst(0, cc1101_GFSK_100_kb, CC1101_CFG_REGISTER);
        break;
    case 0x04:
        write_burst(0, cc1101_MSK_250_kb, CC1101_CFG_REGISTER);
        break;
    case 0x05:
        write_burst(0, cc1101_MSK_500_kb, CC1101_CFG_REGISTER);
        break;
    case 0x06:
        write_burst(0, cc1101_OOK_4_8_kb, CC1101_CFG_REGISTER);
        break;
    default:
        write_burst(0, cc1101_GFSK_100_kb, CC1101_CFG_REGISTER);
        break;
    }
}
void CC1101::set_output_power_level(int8_t dBm)
{
    for (int i = 0; i < CC1101_PATABLE_SIZE; i++)
    {
        if (dBm <= patable_index_power[i])
        {
            write_single_byte(CC1101_FREND0, i);
            Logger::print(LogLevel::DEBUG, "set tx power to %d dBm (#%d)\n", patable_index_power[i], i);
            return;
        }
    }
    Logger::print(LogLevel::WARNING, "invalid output power level %d, ignoring...\n", dBm);
}

void CC1101::reset()
{
    cc1101_select();
    sleep_us(10);
    cc1101_deselect();
    sleep_us(40);
    strobe(CC1101_SRES);
    sleep_ms(1);
}
void CC1101::power_down()
{
    idle_workmode();
    strobe(CC1101_SPWD); // cc1101 Power Down
}
void CC1101::wakeup()
{
    cc1101_select();
    sleep_us(10);
    cc1101_deselect();
    sleep_us(10);
    receive_workmode(); // go to RX Mode
}

void CC1101::idle_workmode()
{
    strobe(CC1101_SIDLE);
    wait_idle();
}
void CC1101::transmit_workmode()
{
    strobe(CC1101_STX); // sends the data over air
}
void CC1101::receive_workmode()
{
    idle_workmode();    // sets to idle first.
    strobe(CC1101_SRX); // writes receive strobe (receive mode)
    wait_rx();
}

void CC1101::fstxon_workmode()
{
    idle_workmode();
    strobe(CC1101_SFSTXON); // writes FSTXON strobe (calibrate freq synthesizer)
    wait_fstxon();
}

bool CC1101::rx_payload_burst(Packet &packet)
{

    uint8_t bytes_in_RXFIFO = read_single_byte(CC1101_RXBYTES);
    // check overflow
    if (bytes_in_RXFIFO & 0x80)
    {

        Logger::print(LogLevel::WARNING, "RX FIFO overflow! Recovering...\n");
        idle_workmode();    // Must go to IDLE first
        flush_rx();         // Flush the bad data
        receive_workmode(); // Restart RX mode
        return false;
    }
    uint8_t num_bytes = bytes_in_RXFIFO & 0x7F; // Strip the overflow bit to get the actual byte count
    // data available
    if (num_bytes > 0)
    {
        read_burst((uint8_t *)&packet, CC1101_RXFIFO_BURST, num_bytes);
        return true;
    }
    return false; // no data to read
}
bool CC1101::packet_available()
{
    if (gpio_get(CC1101_PIN_GDO2)) // if RF package received
    {
        if (read_single_byte(CC1101_IOCFG2) == 0x06) // if sync word detect mode is used
        {
            while (gpio_get(CC1101_PIN_GDO2))
                ; // wait till sync word is fully received
        }
        return true;
    }
    return false;
}

int8_t CC1101::get_live_rssi()
{
    uint8_t raw_rssi = read_single_byte(CC1101_RSSI);
    return rssi_convert(raw_rssi);
}

bool CC1101::get_payload(Packet &packet, int8_t &rssi_dbm, uint8_t &lqi)
{
    if (!rx_payload_burst(packet)) // read package in buffer
    {
        return false; // exit
    }
    rssi_dbm = rssi_convert(packet.payload[packet.header.length - 2]); // converts receiver strength to dBm
    lqi = lqi_convert(packet.payload[packet.header.length - 1]);       // get rf quialtiy indicator
    uint8_t crc = check_crc(packet.payload[packet.header.length - 1]); // get packet CRC
    if (!crc)
    {
        Logger::print(LogLevel::WARNING, "CRC check failed! Recovering...\n");
        idle_workmode();    // Must go to IDLE first
        flush_rx();         // Flush the bad data
        receive_workmode(); // Restart RX mode
        return false;
    }
    return true;
}

bool CC1101::send_packet(Packet &packet)
{
    packet.header.tx_addr = m_address;                   // set sender address
    if (packet.header.length > CC1101_MAX_PACKET_LENGTH) // check if packet size is larger than max payload size (CC1101_MAX_PACKET_LENGTH - 1 byte for length)
    {
        Logger::print(LogLevel::ERROR, "package size overflow\n");
        return false;
    }
    wait_finish_tx();
    write_burst(CC1101_TXFIFO_BURST, (uint8_t *)&packet, packet.header.length + 1); // write data to TX FIFO
    transmit_workmode();                                                            // sends data over air
    return true;
}

CC1101::CC1101(uint8_t freq, uint8_t mode, uint8_t channel, uint8_t address)
    : m_freq(freq), m_mode(mode), m_channel(channel), m_address(address)
{
    init_pins();
    reset();    // cc1101 init reset
    flush_tx(); // flush the TX_fifo content
    sleep_us(100);
    flush_rx(); // flush the RX_fifo content
    sleep_us(100);
    uint8_t partnum = read_single_byte(CC1101_PARTNUM); // reads cc1101 partnumber
    uint8_t version = read_single_byte(CC1101_VERSION); // reads cc1101 version number
    Logger::print(LogLevel::TRACE, "Partnumber: 0x%02X\n", partnum);
    Logger::print(LogLevel::TRACE, "Version   : 0x%02X\n", version);
    // set modulation mode
    set_mode(m_mode);
    // set ISM band
    set_ISM(m_freq);
    // set channel
    set_channel(m_channel);
    // set output power amplifier
    set_output_power_level(-30); // set PA to 0dBm as default
    // set my receiver address
    set_myaddr(m_address); // m_address from EEPROM to global variable
    Logger::print(LogLevel::TRACE, "init done!\n");
    // idle_workmode();
    receive_workmode(); // set cc1101 in receive mode
}
