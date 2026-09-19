#include "DShot600.h"
#include "hardware/clocks.h"
#include "dshot600.pio.h"

DShot600::DShot600(PIO pio, uint sm, uint pin, uint dma_chan)
    : m_pio(pio), m_sm(sm), m_pin(pin), m_dma_chan(dma_chan), m_dma_buffer(0) {}

void DShot600::init()
{
    // 1. Initialize the PIO
    // using static variable to ensure the program is only added once per PIO instance
    uint8_t pio_number;
#if PICO_RP2040
    static int pio_program_offset[2] = {0, 0};
#elif PICO_RP2350
    static int pio_program_offset[3] = {0, 0, 0};
#else
#error "Target chip is not recognized!"
#endif
    if (m_pio == pio0)
        pio_number = 0;
    else if (m_pio == pio1)
        pio_number = 1;
    else
    {
#if PICO_RP2040
        panic("Error: Invalid PIO instance. Must be pio0 or pio1.");
#elif PICO_RP2350
        if (m_pio == pio2 && pio_program_offset[2] == 0)
            pio_number = 2;
        else
            panic("Error: Invalid PIO instance. Must be pio0, pio1 or pio2.");
#endif
    }
    if (pio_program_offset[pio_number] == 0)
        pio_program_offset[pio_number] = pio_add_program(m_pio, &dshot600_program);

    pio_sm_config c = dshot600_program_get_default_config(pio_program_offset[pio_number]);

    // Set PIO pin directions
    pio_gpio_init(m_pio, m_pin);
    pio_sm_set_consecutive_pindirs(m_pio, m_sm, m_pin, 1, true);
    sm_config_set_sideset_pins(&c, m_pin);

    // Shift out configuration:
    // shift_right=false (MSB first), autopull=false (we use explicit pull), 32-bit
    sm_config_set_out_shift(&c, false, false, 32);

    // 12MHz PIO clock for 600kHz DShot (20 cycles per bit)
    float div = (float)clock_get_hz(clk_sys) / 12000000.0f;
    sm_config_set_clkdiv(&c, div);

    pio_sm_init(m_pio, m_sm, pio_program_offset[pio_number], &c);

    // 2. Initialize the DMA
    dma_channel_config dma_c = dma_channel_get_default_config(m_dma_chan);
    channel_config_set_transfer_data_size(&dma_c, DMA_SIZE_32);

    // Allow read pointer to wrap instantly so it loops over the same uint32_t continuously
    channel_config_set_read_increment(&dma_c, true);
    channel_config_set_write_increment(&dma_c, false);
    channel_config_set_ring(&dma_c, false, 2); // Ring on read address, size 2^2 = 4 bytes

    // Pace transfers based on PIO TX FIFO
    channel_config_set_dreq(&dma_c, pio_get_dreq(m_pio, m_sm, true));

    // Configure DMA but don't start yet. Maximum possible transfers.
    // TODO: fix number of transfers for safer method
    dma_channel_configure(
        m_dma_chan,
        &dma_c,
        &m_pio->txf[m_sm], // Write address (PIO TX FIFO)
        &m_dma_buffer,     // Read address (Our looping buffer)
        0xFFFFFFFF,        // Number of transfers (virtually infinite)
        false              // Start immediately? false
    );
}

void DShot600::start()
{
    pio_sm_set_enabled(m_pio, m_sm, true);
    dma_channel_start(m_dma_chan);
}

void DShot600::stop()
{
    dma_channel_abort(m_dma_chan);
    pio_sm_set_enabled(m_pio, m_sm, false);
}

void DShot600::set_throttle(uint16_t throttle, bool telemetry)
{
    uint16_t frame = encode_packet(throttle, telemetry);
    // Shift left by 16 because PIO is configured MSB-first.
    // PIO 'out x, 1' will pull bit 31 first, seamlessly chewing through our 16 bits.
    m_dma_buffer = ((uint32_t)frame) << 16;
}

uint16_t DShot600::encode_packet(uint16_t value, bool telemetry)
{
    // 11 bits data + 1 bit telemetry + 4 bits CRC
    uint16_t packet = (value << 1) | (telemetry ? 1 : 0);

    // Compute CRC
    uint16_t csum = 0;
    uint16_t csum_data = packet;
    csum = (csum_data ^ (csum_data >> 4) ^ (csum_data >> 8)) & 0x0F;

    return (packet << 4) | csum;
}