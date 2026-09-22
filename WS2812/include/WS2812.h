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
    struct __attribute__((packed)) RGB
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;

        constexpr RGB(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
        constexpr RGB() : r(0), g(0), b(0) {}
    };

public:
    /**
     * Constructs a WS2812 instance.
     *
     * @param pio PIO instance
     * @param sm State Machine index (0-3)
     * @param pin GPIO pin connected to the WS2812 data line
     * @param num_leds Number of LEDs in the WS2812 strip
     * @param dma_chan DMA channel to use for data transfer (defaults to -1 for auto-claim)
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
     * Sets the RGB color of a specific LED in the buffer using an RGB struct.
     *
     * @param index The 0-based index of the LED
     * @param color The RGB color struct
     */
    void set_pixel_color(uint index, RGB color);

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

namespace WS2812Colors
{
    constexpr WS2812::RGB BLACK{0x00, 0x00, 0x00};
    constexpr WS2812::RGB WHITE{0xFF, 0xFF, 0xFF};
    constexpr WS2812::RGB SILVER{0xC0, 0xC0, 0xC0};
    constexpr WS2812::RGB GRAY{0x80, 0x80, 0x80};

    constexpr WS2812::RGB RED{0xFF, 0x00, 0x00};
    constexpr WS2812::RGB GREEN{0x00, 0xFF, 0x00};
    constexpr WS2812::RGB BLUE{0x00, 0x00, 0xFF};
    constexpr WS2812::RGB YELLOW{0xFF, 0xFF, 0x00};
    constexpr WS2812::RGB CYAN{0x00, 0xFF, 0xFF};
    constexpr WS2812::RGB MAGENTA{0xFF, 0x00, 0xFF};
    constexpr WS2812::RGB ORANGE{0xFF, 0xA5, 0x00};
    constexpr WS2812::RGB PINK{0xFF, 0xC0, 0xCB};
    constexpr WS2812::RGB BROWN{0xA5, 0x2A, 0x2A};
    constexpr WS2812::RGB GOLD{0xFF, 0xD7, 0x00};

    // Half Intensity Colors
    constexpr WS2812::RGB MAROON{0x80, 0x00, 0x00};
    constexpr WS2812::RGB OLIVE{0x80, 0x80, 0x00};
    constexpr WS2812::RGB DARK_GREEN{0x00, 0x80, 0x00}; // Standard web "Green"
    constexpr WS2812::RGB TEAL{0x00, 0x80, 0x80};
    constexpr WS2812::RGB NAVY{0x00, 0x00, 0x80};
    constexpr WS2812::RGB PURPLE{0x80, 0x00, 0x80};
}