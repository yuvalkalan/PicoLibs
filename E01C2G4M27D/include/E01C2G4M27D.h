#pragma once
#include "HighSPI.h"
#include "hardware/timer.h"
#include <string.h>
#include "E01C2G4M27D_REG.h"

#define SPI_SPEED 10'000'000 // 10MHZ

class E01C2G4M27D : public HighSPI
{
private:
    uint m_ce, m_irq;
    uint8_t m_payload_size;

public:
    // Constructor maps the hardware SPI instance and GPIO pins
    E01C2G4M27D(spi_inst_t *spi, uint ce, uint csn, uint sck, uint mosi, uint miso, uint irq);

    void begin();

    // Configuration
    void set_channel(uint8_t channel); // 0-125
    void set_PA_level(uint8_t level);  // 0: Min, 3: Max
    void set_data_rate(uint8_t rate);  // 0: 1Mbps, 1: 2Mbps, 2: 250kbps

    // Pipe addressing
    void open_writing_pipe(const uint8_t *address);
    void open_reading_pipe(uint8_t number, const uint8_t *address);

    // Operation
    void start_listening();
    void stop_listening();

    // Data transfer
    bool write(const void *buf, uint8_t len);
    bool available();
    void read(void *buf, uint8_t len);

    // Quality information
    uint8_t get_observe_tx(); // Returns Reg 0x08 (PLOS_CNT and ARC_CNT)
    bool get_RPD();           // Returns Reg 0x09 bit 0 (Received Power Detector)

private: // helper functions (use as macros)
    __always_inline uint8_t readRegister(uint8_t reg) { return read_single_byte(CMD_R_REGISTER | (reg & 0x1F)); }
    __always_inline void writeRegister(uint8_t reg, uint8_t data) { write_single_byte(CMD_W_REGISTER | (reg & 0x1F), data); }
    __always_inline void readRegisterMulti(uint8_t reg, uint8_t *buffer, size_t bytes) { read_burst(CMD_R_REGISTER | (reg & 0x1F), buffer, bytes); }
    __always_inline void writeRegisterMulti(uint8_t reg, const uint8_t *buffer, size_t bytes) { write_burst(CMD_W_REGISTER | (reg & 0x1F), buffer, bytes); }
    __always_inline uint8_t get_status() { return strobe(0xFF); }
    __always_inline void ceLow() { gpio_put(m_ce, 0); }
    __always_inline void ceHigh() { gpio_put(m_ce, 1); }
    __always_inline void flush_tx() { strobe(CMD_FLUSH_TX); }
    __always_inline void flush_rx() { strobe(CMD_FLUSH_RX); }
};