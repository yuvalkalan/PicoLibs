#pragma once

#include "stdio.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "configuration.h"
#include <string>
#include <string.h>
#include "Logger.h"

#define CC1101_SPI_PORT spi1
#define CC1101_PIN_SCK 10
#define CC1101_PIN_MOSI 11
#define CC1101_PIN_MISO 12
#define CC1101_PIN_CS 13
#define CC1101_PIN_GDO0 15
#define CC1101_PIN_GDO2 14
#define CC1101_SPI_BAUDRATE 10 * 1000 * 1000 // 5MHZ

// cc1101 gpio macros
#define cc1101_select() gpio_put(CC1101_PIN_CS, 0)   //  Select (SPI) CC1101
#define cc1101_deselect() gpio_put(CC1101_PIN_CS, 1) // Deselect (SPI) CC1101

// cc1101 softwate macros
#define in_idle_mode() (read_single_byte(CC1101_MARCSTATE) & 0x1F) == 0x01
#define in_rx_mode() (read_single_byte(CC1101_MARCSTATE) & 0x1F) == 0x0D
#define in_tx_mode() (read_single_byte(CC1101_MARCSTATE) & 0x1F) == 0x13
#define in_fstxon_mode() (read_single_byte(CC1101_MARCSTATE) & 0x1F) == 0x12

#define wait_idle() while (!in_idle_mode())     // Wait until enter idle mode
#define wait_rx() while (!in_rx_mode())         // Wait until enter rx mode
#define wait_tx() while (!in_tx_mode())         // Wait until enter rx mode
#define wait_fstxon() while (!in_fstxon_mode()) // Wait until enter FSTXON mode

#define wait_finish_tx() while (!(in_idle_mode() || in_fstxon_mode())) // can set to tx only if current in idle or fstxon mode

#define flush_rx() strobe(CC1101_SFRX)
#define flush_tx() strobe(CC1101_SFTX)

int8_t rssi_convert(uint8_t Rssi_hex);
uint8_t lqi_convert(uint8_t lqi);

struct __attribute__((packed)) PacketHeader
{
    uint8_t length;  // length of payload + header (do not move this!)
    uint8_t rx_addr; // receiver address (used for filtering in hardware, do not move this!)
    uint8_t tx_addr;
};

struct __attribute__((packed)) Packet
{
    PacketHeader header;
    uint8_t payload[CC1101_FIFOBUFFER - sizeof(header)] = {0};
    std::string to_string() const
    {
        char buffer[256];
        sprintf(buffer, "(%d)\t %02X -> %02X", header.length, header.tx_addr, header.rx_addr);
        return std::string(buffer) + std::string((char *)payload);
    }
};

class CC1101
{
protected: // variables
    uint8_t m_freq;
    uint8_t m_mode;
    uint8_t m_channel;
    uint8_t m_address;

protected: // low spi api
    void strobe(uint8_t cmd);
    void write_single_byte(uint8_t address, uint8_t data);
    uint8_t read_single_byte(uint8_t address);
    void write_burst(uint8_t reg_addr, uint8_t *buffer, size_t size);
    void read_burst(uint8_t *buffer, uint8_t reg_addr, uint8_t len);
    void init_pins();

protected: // hardware api
    void power_down();
    void wakeup();
    void reset();

protected: // software configuration api
    void set_channel(uint8_t channel);
    void set_ISM(uint8_t ism_freq);
    void set_mode(uint8_t mode);
    void set_output_power_level(int8_t dBm);

public: // work modes
    void idle_workmode();
    void transmit_workmode();
    void receive_workmode();
    void fstxon_workmode();

protected: // transmit and receive sub-functions
    bool rx_payload_burst(Packet &packet);
    int8_t get_live_rssi();

public: // transmit and receive functions
    void set_myaddr(uint8_t addr);
    bool send_packet(Packet &packet);
    bool get_payload(Packet &packet, int8_t &rssi_dbm, uint8_t &lqi);
    bool packet_available();

public:
    CC1101(uint8_t freq, uint8_t mode, uint8_t channel, uint8_t address);
};
