#include "WS2812.h"
#include "hardware/clocks.h"
#include "WS2812.pio.h"
#include "HighPIO.h"
#include <string.h>

WS2812::WS2812(PIO pio, uint sm, uint pin, uint num_leds, int dma_chan)
    : m_pio(pio), m_sm(sm), m_pin(pin), m_dma_chan(dma_chan == -1 ? dma_claim_unused_channel(true) : dma_chan), m_num_leds(num_leds)
{
    m_dma_buffer = new uint32_t[m_num_leds];
    clear();
}

WS2812::~WS2812()
{
    delete[] m_dma_buffer;
}

void WS2812::init()
{
    // 1. Initialize the PIO
    uint offset = pio_add_program_once(m_pio, &ws2812_program);

    pio_sm_config c = ws2812_program_get_default_config(offset);

    // Set PIO pin directions
    pio_gpio_init(m_pio, m_pin);
    pio_sm_set_consecutive_pindirs(m_pio, m_sm, m_pin, 1, true);
    sm_config_set_sideset_pins(&c, m_pin);

    // Shift out configuration:
    // shift_right=false (MSB first), autopull=true, 24-bit threshold
    sm_config_set_out_shift(&c, false, true, 24);

    // 8MHz PIO clock for 800kHz WS2812 (10 cycles per bit)
    float div = (float)clock_get_hz(clk_sys) / 8000000.0f;
    sm_config_set_clkdiv(&c, div);

    // Join FIFOs to give the state machine more TX capacity
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

    pio_sm_init(m_pio, m_sm, offset, &c);
    pio_sm_set_enabled(m_pio, m_sm, true);

    // 2. Initialize the DMA
    dma_channel_config dma_c = dma_channel_get_default_config(m_dma_chan);
    channel_config_set_transfer_data_size(&dma_c, DMA_SIZE_32);

    // Increment read address to traverse the buffer, do not increment write address (TX FIFO)
    channel_config_set_read_increment(&dma_c, true);
    channel_config_set_write_increment(&dma_c, false);

    // Pace transfers based on PIO TX FIFO
    channel_config_set_dreq(&dma_c, pio_get_dreq(m_pio, m_sm, true));

    dma_channel_configure(
        m_dma_chan, &dma_c, &m_pio->txf[m_sm], // Write address (PIO TX FIFO)
        m_dma_buffer,                          // Read address
        m_num_leds,                            // Number of transfers
        false                                  // Start immediately? false
    );
}

void WS2812::show()
{
    // Wait for the previous DMA transfer to finish before starting a new one
    dma_channel_wait_for_finish_blocking(m_dma_chan);

    // Set the read address back to the start of the buffer and trigger the transfer
    dma_channel_set_read_addr(m_dma_chan, m_dma_buffer, true);
}

void WS2812::set_pixel_color(uint index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index >= m_num_leds)
        return;

    // WS2812 protocol requires GRB order.
    // Since we configured the PIO for MSB-first out shift with a 24-bit autopull threshold,
    // we place the 24 bits of color data in the most significant bits of the 32-bit word.
    m_dma_buffer[index] = ((uint32_t)g << 24) | ((uint32_t)r << 16) | ((uint32_t)b << 8);
}

void WS2812::set_pixel_color(uint index, RGB color)
{
    set_pixel_color(index, color.r, color.g, color.b);
}

void WS2812::clear()
{
    memset(m_dma_buffer, 0, m_num_leds * sizeof(uint32_t));
}