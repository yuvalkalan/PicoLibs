#include "HighSPI.h"

// using these functions as macros
// __always_inline void HighSPI::select() { gpio_put(m_csn, 0); }
// __always_inline void HighSPI::deselect() { gpio_put(m_csn, 1); }

uint8_t HighSPI::strobe(uint8_t cmd)
{
    select();
    uint8_t rx = 0;
    spi_write_read_blocking(m_spi, &cmd, &rx, 1);
    deselect();
    return rx;
}
void HighSPI::write_single_byte(uint8_t address, const uint8_t data)
{
    select();
    uint8_t address_byte = address;
    spi_write_blocking(m_spi, &address_byte, 1);
    spi_write_blocking(m_spi, &data, 1);
    deselect();
}
uint8_t HighSPI::read_single_byte(uint8_t address)
{
    uint8_t data;
    select();
    uint8_t address_byte = address;
    spi_write_blocking(m_spi, &address_byte, 1);
    spi_read_blocking(m_spi, 0x00, &data, 1);
    deselect();
    return data;
}
void HighSPI::write_burst(uint8_t address, const uint8_t *buffer, size_t bytes)
{
    uint8_t addr = address;
    select();
    spi_write_blocking(m_spi, &addr, 1);
    spi_write_blocking(m_spi, buffer, bytes);
    deselect();
}

void HighSPI::read_burst(uint8_t address, uint8_t *buffer, uint8_t bytes)
{
    uint8_t addr = address;
    select();
    spi_write_blocking(m_spi, &addr, 1);           // Send register address
    spi_read_blocking(m_spi, 0x00, buffer, bytes); // Read result
    deselect();
}

HighSPI::HighSPI(spi_inst_t *spi, uint miso, uint csn, uint sck, uint mosi, uint baudrate)
    : m_spi(spi), m_miso(miso), m_csn(csn), m_sck(sck), m_mosi(mosi), m_baudrate(baudrate)
{
}

void HighSPI::init_spi()
{
    // Initialize SPI port
    spi_init(m_spi, m_baudrate);
    gpio_set_function(m_miso, GPIO_FUNC_SPI);
    gpio_set_function(m_mosi, GPIO_FUNC_SPI);
    gpio_set_function(m_sck, GPIO_FUNC_SPI);

    // Configure Chip Select (CS) pin
    gpio_init(m_csn);
    gpio_set_dir(m_csn, GPIO_OUT);
    deselect();
}