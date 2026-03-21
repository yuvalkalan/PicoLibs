#include "CC1101.h"

// Patable index: -30  -20- -15  -10   0    5    7    10 dBm
static uint8_t patable_power_315[8] = {0x17, 0x1D, 0x26, 0x69, 0x51, 0x86, 0xCC, 0xC3};
static uint8_t patable_power_433[8] = {0x6C, 0x1C, 0x06, 0x3A, 0x51, 0x85, 0xC8, 0xC0};
static uint8_t patable_power_868[8] = {0x03, 0x17, 0x1D, 0x26, 0x50, 0x86, 0xCD, 0xC0};
static uint8_t patable_power_915[8] = {0x0B, 0x1B, 0x6D, 0x67, 0x50, 0x85, 0xC9, 0xC1};

// byte operators macros
#define bitRead(value, bit) (((value) >> (bit)) & 0x01) // bitRead function from arduino

void fifo_erase(uint8_t *buffer)
{
    memset(buffer, 0, CC1101_FIFOBUFFER); // erased the RX_fifo array content to "0"
}
int8_t rssi_convert(uint8_t Rssi_hex)
{
    int8_t rssi_dbm;
    int16_t Rssi_dec;

    Rssi_dec = Rssi_hex; // convert unsigned to signed

    if (Rssi_dec >= 128)
    {
        rssi_dbm = ((Rssi_dec - 256) / 2) - CC1101_RSSI_OFFSET_868MHZ;
    }
    else
    {
        if (Rssi_dec < 128)
        {
            rssi_dbm = ((Rssi_dec) / 2) - CC1101_RSSI_OFFSET_868MHZ;
        }
    }
    return rssi_dbm;
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
    write_single_byte(CC1101_ADDR, addr); // stores MyAddr in the cc1101
}
void CC1101::set_channel(uint8_t channel)
{
    write_single_byte(CC1101_CHANNR, channel); // stores the new channel # in the cc1101
}
void CC1101::set_ISM(uint8_t ism_freq)
{
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
    uint8_t pa = 0xC0;

    if (dBm <= -30)
        pa = 0x00;
    else if (dBm <= -20)
        pa = 0x01;
    else if (dBm <= -15)
        pa = 0x02;
    else if (dBm <= -10)
        pa = 0x03;
    else if (dBm <= 0)
        pa = 0x04;
    else if (dBm <= 5)
        pa = 0x05;
    else if (dBm <= 7)
        pa = 0x06;
    else if (dBm <= 10)
        pa = 0x07;

    write_single_byte(CC1101_FREND0, pa);
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
    uint8_t marcstate;
    strobe(CC1101_SIDLE);
    wait_idle();
    sleep_us(100);
}
void CC1101::transmit_workmode()
{
    idle_workmode();    // sets to idle first. TODO: try to remove
    strobe(CC1101_STX); // sends the data over air
    wait_idle();
    sleep_us(100);
}
void CC1101::receive_workmode()
{
    idle_workmode();    // sets to idle first.
    strobe(CC1101_SRX); // writes receive strobe (receive mode)
    wait_rx();
    sleep_us(100);
}

void CC1101::tx_payload_burst(uint8_t rx_addr, uint8_t length)
{
    m_tx_buffer[0] = length - 1;
    m_tx_buffer[1] = rx_addr;
    m_tx_buffer[2] = m_address;
    write_burst(CC1101_TXFIFO_BURST, m_tx_buffer, length); // writes TX_Buffer +1 because of pktlen must be also transfered
    // printf("sending data...\n");
}
bool CC1101::rx_payload_burst(uint8_t &pktlen)
{
    uint8_t bytes_in_RXFIFO = read_single_byte(CC1101_RXBYTES); // reads the number of bytes in RXFIFO
    bool res = false;
    if ((bytes_in_RXFIFO & 0x7F) && !(bytes_in_RXFIFO & 0x80)) // if bytes in buffer and no RX Overflow
    {
        // printf("have data!\n");
        read_burst(rx_buffer, CC1101_RXFIFO_BURST, bytes_in_RXFIFO);
        pktlen = rx_buffer[0];
        // printf("receive data!\n");
        res = true;
    }
    else
    {
        printf("did not receive data!\n");
        idle_workmode(); // set to IDLE
        flush_rx();      // flush RX Buffer
        sleep_us(100);
        receive_workmode(); // set to receive mode
    }
    return res;
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
void CC1101::sent_acknowledge(uint8_t tx_addr)
{
    uint8_t pktlen = 0x06; // complete Pktlen for ACK packet

    // fill buffer with ACK Payload
    m_tx_buffer[3] = 'A';
    m_tx_buffer[4] = 'c';
    m_tx_buffer[5] = 'k';

    tx_payload_burst(tx_addr, pktlen); // load payload to CC1101
    transmit_workmode();               // sent package over the air
    receive_workmode();                // set CC1101 in receive mode

    // printf("Ack_sent!\r\n");
}
bool CC1101::check_acknowledge(uint8_t pktlen, uint8_t sender)
{
    int8_t rssi_dbm;
    uint8_t crc, lqi;

    if ((pktlen == 0x05 && rx_buffer[1] == m_address || rx_buffer[1] == CC1101_BROADCAST_ADDRESS) && rx_buffer[2] == sender && rx_buffer[3] == 'A' && rx_buffer[4] == 'c' && rx_buffer[5] == 'k') // acknowledge received!
    {
        if (rx_buffer[1] == CC1101_BROADCAST_ADDRESS)
        { // if receiver address CC1101_BROADCAST_ADDRESS skip acknowledge
            // printf("BROADCAST ACK\r\n");
            return false;
        }
        rssi_dbm = rssi_convert(rx_buffer[pktlen + 1]);
        lqi = lqi_convert(rx_buffer[pktlen + 2]);
        crc = check_crc(lqi);

        // printf("ACK! ");
        // printf("RSSI:%i ", rssi_dbm);
        // printf("LQI:0x%02X ", lqi);
        // printf("CRC:0x%02X\r\n", crc);
        return true;
    }
    return false;
}
bool CC1101::get_payload(uint8_t &pktlen, uint8_t &sender, int8_t &rssi_dbm, uint8_t &lqi)
{
    uint8_t crc;

    fifo_erase(rx_buffer);         // delete rx_fifo bufffer
    if (!rx_payload_burst(pktlen)) // read package in buffer
    {
        fifo_erase(rx_buffer); // delete rx_fifo bufffer
        return false;          // exit
    }
    else
    {
        sender = rx_buffer[2];
        if (check_acknowledge(pktlen, sender)) // acknowlage received?
        {
            printf("ACK received!\r\n");
            fifo_erase(rx_buffer); // delete rx_fifo bufffer
            return false;          // Ack received -> finished
        }
        else // real data, and sent acknowladge
        {
            printf("Data received!\r\n");
            rssi_dbm = rssi_convert(rx_buffer[pktlen + 1]); // converts receiver strength to dBm
            lqi = lqi_convert(rx_buffer[pktlen + 2]);       // get rf quialtiy indicator
            crc = check_crc(lqi);                           // get packet CRC
            // if (true)
            // {                                                 // debug output messages
            //     if (rx_buffer[1] == CC1101_BROADCAST_ADDRESS) // if my receiver address is CC1101_BROADCAST_ADDRESS
            //     {
            //         printf("BROADCAST message\r\n");
            //     }
            //     printf("RX_FIFO:");
            //     for (uint8_t i = 0; i < pktlen + 1; i++) // showes rx_buffer for debug
            //     {
            //         printf("0x%02X ", rx_buffer[i]);
            //     }
            //     printf("| 0x%02X 0x%02X |", rx_buffer[pktlen + 1], rx_buffer[pktlen + 2]);
            //     printf("\r\n");
            //     printf("RSSI:%d ", rssi_dbm);
            //     printf("LQI:");
            //     printf("0x%02X ", lqi);
            //     printf("CRC:");
            //     printf("0x%02X ", crc);
            //     printf("\r\n");
            // }
            if (rx_buffer[1] != CC1101_BROADCAST_ADDRESS) // send only ack if no CC1101_BROADCAST_ADDRESS
            {
                sleep_us(200);            // wait some time to let the sender change to rx mode
                sent_acknowledge(sender); // sending acknowlage to sender!
            }
            return true;
        }
        return false;
    }
}

bool CC1101::sent_packet(uint8_t rx_addr, uint8_t *data, uint8_t pktlen, uint8_t tx_retries)
{
    uint8_t pktlen_ack, rssi, lqi; // default package len for ACK
    uint8_t tx_retries_count = 0;
    uint16_t ack_wait_counter = 0;
    pktlen += 3; // real data size (contain length, rx address and tx address)
    if (pktlen > (CC1101_FIFOBUFFER - 1))
    {
        printf("ERROR: package size overflow\r\n");
        return false;
    }

    // set tx buffer
    memcpy((m_tx_buffer + 3), data, pktlen - 3);

    do // sent package out with retries
    {
        tx_payload_burst(rx_addr, pktlen); // loads the data in cc1101 buffer
        transmit_workmode();               // sents data over air
        receive_workmode();                // receive mode

        if (rx_addr == CC1101_BROADCAST_ADDRESS)
        {                // no wait acknowledge if sent to broadcast address or tx_retries = 0
            return true; // successful sent to CC1101_BROADCAST_ADDRESS
        }

        while (ack_wait_counter < CC1101_ACK_TIMEOUT) // wait for an acknowledge
        {
            if (packet_available()) // if RF package received check package acknowge
            {
                fifo_erase(rx_buffer);                      // erase RX software buffer
                rx_payload_burst(pktlen_ack);               // reads package in buffer
                if (check_acknowledge(pktlen_ack, rx_addr)) // check if received message is an acknowledge from client
                    return true;                            // package successfully sent
            }
            else
            {
                ack_wait_counter++; // increment ACK wait counter
                sleep_ms(1);        // delay to give receiver time
            }
        }

        ack_wait_counter = 0; // resets the ACK_Timeout
        tx_retries_count++;   // increase tx retry counter
        // printf(" #:0x%02X \r\n", tx_retries_count); // debug output messages

    } while (tx_retries_count <= tx_retries); // while count of retries is reaches

    return false; // sent failed. too many retries
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
    printf("Partnumber: 0x%02X\n", partnum);
    printf("Version   : 0x%02X\n", version);
    // set modulation mode
    set_mode(m_mode);
    // set ISM band
    set_ISM(m_freq);
    // set channel
    set_channel(m_channel);
    // set output power amplifier
    set_output_power_level(10); // set PA to 0dBm as default
    // set my receiver address
    set_myaddr(m_address); // m_address from EEPROM to global variable
    printf("init done!\n");
    receive_workmode(); // set cc1101 in receive mode
}
