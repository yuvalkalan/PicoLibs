#pragma once
#include "pico/stdlib.h"
#include "hardware/spi.h"

/* high spi api*/

class HighSPI
{
protected:
    spi_inst_t *m_spi;                 // SPI instance
    uint m_baudrate;                   // spi baudrate speed
    uint m_miso, m_csn, m_sck, m_mosi; // gpio pins

protected:
    uint8_t strobe(uint8_t cmd);
    void write_single_byte(uint8_t address, const uint8_t data);
    uint8_t read_single_byte(uint8_t address);
    void write_burst(uint8_t address, const uint8_t *buffer, size_t bytes);
    void read_burst(uint8_t address, uint8_t *buffer, uint8_t bytes);
    void init_spi();

protected: // helper functions (use as macros)
    __always_inline void select() { gpio_put(m_csn, 0); }
    __always_inline void deselect() { gpio_put(m_csn, 1); }

public:
    HighSPI(spi_inst_t *spi, uint miso, uint csn, uint sck, uint mosi, uint baudrate);
};
