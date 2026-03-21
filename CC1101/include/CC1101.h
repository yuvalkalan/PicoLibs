#pragma once

#include "stdio.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "configuration.h"
#include <string.h>

#define CC1101_SPI_PORT spi1
#define CC1101_PIN_SCK 10
#define CC1101_PIN_MOSI 11
#define CC1101_PIN_MISO 12
#define CC1101_PIN_CS 13
#define CC1101_PIN_GDO0 15
#define CC1101_PIN_GDO2 14
#define CC1101_SPI_BAUDRATE 5 * 1000 * 1000 // 5MHZ

// cc1101 gpio macros
#define cc1101_select() gpio_put(CC1101_PIN_CS, 0)   //  Select (SPI) CC1101
#define cc1101_deselect() gpio_put(CC1101_PIN_CS, 1) // Deselect (SPI) CC1101
// cc1101 softwate macros
#define wait_idle() while ((read_single_byte(CC1101_MARCSTATE) & 0x1F) != 0x01) // Wait untile enter idle mode
#define wait_rx() while ((read_single_byte(CC1101_MARCSTATE) & 0x1F) != 0x0D)   // Wait untile enter rx mode
#define flush_rx() strobe(CC1101_SFRX)
#define flush_tx() strobe(CC1101_SFTX)

int8_t rssi_convert(uint8_t Rssi_hex);
uint8_t lqi_convert(uint8_t lqi);

class CC1101
{
protected: // variables
    uint8_t m_freq;
    uint8_t m_mode;
    uint8_t m_channel;
    uint8_t m_address;
    uint8_t m_tx_buffer[CC1101_FIFOBUFFER];

public:
    uint8_t rx_buffer[CC1101_FIFOBUFFER];

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
    void set_myaddr(uint8_t addr);
    void set_channel(uint8_t channel);
    void set_ISM(uint8_t ism_freq);
    void set_mode(uint8_t mode);
    void set_output_power_level(int8_t dBm);

protected: // work modes
    void idle_workmode();
    void transmit_workmode();
    void receive_workmode();

protected: // transmit and receive sub-functions
    void tx_payload_burst(uint8_t rx_addr, uint8_t length);
    bool rx_payload_burst(uint8_t &pktlen);
    bool check_acknowledge(uint8_t pktlen, uint8_t sender);
    void sent_acknowledge(uint8_t tx_addr);

public: // transmit and receive functions
    bool sent_packet(uint8_t rx_addr, uint8_t *tx_buffer, uint8_t pktlen, uint8_t tx_retries);
    bool get_payload(uint8_t &pktlen, uint8_t &sender, int8_t &rssi_dbm, uint8_t &lqi);
    bool packet_available();

public:
    CC1101(uint8_t freq, uint8_t mode, uint8_t channel, uint8_t address);
};