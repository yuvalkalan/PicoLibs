#pragma once

#include "stdio.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "configuration.h"
#include <string>
#include <string.h>
#include "Logger.h"
#include "HighSPI.h"

#define CC1101_SPI_BAUDRATE 10'000'000 // 10MHZ

class CC1101 : public HighSPI
{
public:
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

protected: // hardware pins variables
    uint m_gdo2;
    uint m_gdo0;

protected: // variables
    uint8_t m_freq;
    uint8_t m_mode;
    uint8_t m_channel;
    uint8_t m_address;

protected: // hardware api
    void power_down();
    void wakeup();
    void reset();

protected: // software configuration api
    void set_channel(uint8_t channel);
    void set_ISM(uint8_t ism_freq);
    void set_mode(uint8_t mode);

public: // work modes
    void idle_workmode();
    void transmit_workmode();
    void receive_workmode();
    void fstxon_workmode();

protected: // transmit and receive sub-functions
    bool rx_payload_burst(Packet &packet);
    int8_t get_live_rssi();

protected: // helper functions (use as macros)
    __always_inline bool in_idle_mode() { return (read_single_byte(CC1101_MARCSTATE | CC1101_READ_SINGLE_BYTE) & 0x1F) == 0x01; }
    __always_inline bool in_rx_mode() { return (read_single_byte(CC1101_MARCSTATE | CC1101_READ_SINGLE_BYTE) & 0x1F) == 0x0D; }
    __always_inline bool in_tx_mode() { return (read_single_byte(CC1101_MARCSTATE | CC1101_READ_SINGLE_BYTE) & 0x1F) == 0x13; }
    __always_inline bool in_fstxon_mode() { return (read_single_byte(CC1101_MARCSTATE | CC1101_READ_SINGLE_BYTE) & 0x1F) == 0x12; }

    __always_inline void wait_idle() // Wait until enter idle mode
    {
        while (!in_idle_mode())
            ;
    }
    __always_inline void wait_rx() // Wait until enter rx mode
    {
        while (!in_rx_mode())
            ;
    }
    __always_inline void wait_tx() // Wait until enter tx mode
    {
        while (!in_tx_mode())
            ;
    }
    __always_inline void wait_fstxon() // Wait until enter FSTXON mode
    {
        while (!in_fstxon_mode())
            ;
    }
    __always_inline void wait_finish_tx() // can set to tx only if current in idle or fstxon mode
    {
        while (!(in_idle_mode() || in_fstxon_mode()))
            ;
    }

    __always_inline void flush_rx()
    {
        strobe(CC1101_SFRX);
    }
    __always_inline void flush_tx()
    {
        strobe(CC1101_SFTX);
    }

public: // transmit and receive functions
    void set_myaddr(uint8_t addr);
    bool send_packet(Packet &packet);
    bool get_payload(Packet &packet, int8_t &rssi_dbm, uint8_t &lqi);
    bool packet_available();
    void set_output_power_level(int8_t dBm);

public: // static convertors function
    static int8_t rssi_convert(uint8_t Rssi_hex);
    static uint8_t lqi_convert(uint8_t lqi);
    static uint8_t check_crc(uint8_t lqi);

public:
    void init();
    CC1101(spi_inst_t *spi, uint miso, uint csn, uint sck, uint mosi, uint gdo2, uint gdo0, uint8_t freq, uint8_t mode, uint8_t channel, uint8_t address);
};
