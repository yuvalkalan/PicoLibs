#pragma once
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <cstring>

/* high i2c api*/

class HighI2C
{
protected:
    i2c_inst_t *m_i2c; // i2c instance
    uint8_t m_device_address;
    uint m_baudrate;   // i2c baudrate speed
    uint m_sda, m_scl; // gpio pins

protected:
    void write_command(uint8_t cmd);
    void write_single_byte(uint8_t reg_address, const uint8_t data);
    uint8_t read_single_byte(uint8_t reg_address);
    void write_burst(uint8_t reg_address, const uint8_t *buffer, size_t bytes, bool single_buffer = true);
    void read_burst(uint8_t reg_address, uint8_t *buffer, size_t bytes);
    void init_i2c();

public:
    HighI2C(i2c_inst_t *i2c, uint8_t device_address, uint sda, uint scl, uint baudrate);
};
