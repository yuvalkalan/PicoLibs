#pragma once

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

/**
 * A C++ library for controlling WS2812 LED strips using PIO and DMA.
 *
 * This class implements a non-blocking WS2812 driver. It utilizes a dynamically
 * allocated buffer to store the color states of all LEDs. The DMA controller
 * is used to automatically feed pixel data into the PIO state machine, freeing
 * the CPU from bit-banging and timing constraints.
 */
class WS2812
{
public:
    /**
     * Constructs a WS2812 instance.
     *
     * @param pio PIO instance (pio0 or pio1)
     * @param sm State Machine index (0-3)
     * @param pin GPIO pin connected to the WS2812 data line
     * @param dma_chan DMA channel to use for data transfer
     * @param num_leds Number of LEDs in the WS2812 strip
     */
    WS2812(PIO pio, uint sm, uint pin, uint num_leds, int dma_chan = -1);

    /**
     * Destroys the WS2812 instance and frees the internal pixel buffer.
     */
    ~WS2812();

    /**
     * Initializes the PIO, DMA, and GPIO peripherals.
     * Configures the PIO program offset safely across multiple instances.
     */
    void init();

    /**
     * Triggers the DMA to send the current buffer to the LED strip.
     * This function is non-blocking and returns immediately.
     */
    void show();

    /**
     * Sets the RGB color of a specific LED in the buffer.
     *
     * @param index The 0-based index of the LED
     * @param r Red intensity (0-255)
     * @param g Green intensity (0-255)
     * @param b Blue intensity (0-255)
     */
    void set_pixel_color(uint index, uint8_t r, uint8_t g, uint8_t b);

    /**
     * Clears all pixels in the buffer (sets them to off/black).
     */
    void clear();

private:
    PIO m_pio;
    uint m_sm;
    uint m_pin;
    uint m_dma_chan;
    uint m_num_leds;
    uint32_t *m_dma_buffer;
};