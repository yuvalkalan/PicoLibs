#include "HighI2C.h"

void HighI2C::write_command(uint8_t cmd)
{
    // Write a single command byte with no register address
    i2c_write_blocking(m_i2c, m_device_address, &cmd, 1, false);
}

void HighI2C::write_single_byte(uint8_t reg_address, const uint8_t data)
{
    // Pack the register address and data into a single buffer
    uint8_t buffer[2] = {reg_address, data};
    i2c_write_blocking(m_i2c, m_device_address, buffer, 2, false);
}

uint8_t HighI2C::read_single_byte(uint8_t reg_address)
{
    uint8_t data;
    // Write register address, keeping control of the bus (nostop = true)
    i2c_write_blocking(m_i2c, m_device_address, &reg_address, 1, true);
    // Read the data byte (nostop = false to release the bus)
    i2c_read_blocking(m_i2c, m_device_address, &data, 1, false);
    return data;
}

void HighI2C::write_burst(uint8_t reg_address, const uint8_t *buffer, size_t bytes, bool single_buffer)
{

    // this method use the single-buffer approach - dynamically allocate memory and send data all at once. safer but slower.
    if (single_buffer)
    {

        uint8_t *temp = new uint8_t[bytes + 1]; // Dynamically allocate memory for the register + payload
        temp[0] = reg_address;                  // Set the register pointer
        memcpy(temp + 1, buffer, bytes);        // Copy the entire buffer payload in one highly optimized step
        // Send everything in ONE single continuous I2C transaction
        i2c_write_blocking(m_i2c, m_device_address, temp, bytes + 1, false);
        delete[] temp;
    }
    else
    {
        // Write register address, keeping control of the bus (nostop = true)
        i2c_write_blocking(m_i2c, m_device_address, &reg_address, 1, true);
        // Write the buffer payload
        i2c_write_blocking(m_i2c, m_device_address, buffer, bytes, false);
    }
}

void HighI2C::read_burst(uint8_t reg_address, uint8_t *buffer, size_t bytes)
{
    // Write register address, keeping control of the bus (nostop = true)
    i2c_write_blocking(m_i2c, m_device_address, &reg_address, 1, true);
    // Read result into buffer
    i2c_read_blocking(m_i2c, m_device_address, buffer, bytes, false);
}

HighI2C::HighI2C(i2c_inst_t *i2c, uint8_t device_address, uint sda, uint scl, uint baudrate)
    : m_i2c(i2c), m_device_address(device_address), m_sda(sda), m_scl(scl), m_baudrate(baudrate)
{
}

void HighI2C::init_i2c()
{
    // Initialize I2C port
    i2c_init(m_i2c, m_baudrate);

    // Configure GPIO pins for I2C
    gpio_set_function(m_sda, GPIO_FUNC_I2C);
    gpio_set_function(m_scl, GPIO_FUNC_I2C);

    // Enable internal pull-ups (required for I2C lines)
    gpio_pull_up(m_sda);
    gpio_pull_up(m_scl);
}