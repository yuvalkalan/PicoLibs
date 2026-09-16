#include "ADS1115.h"

ADS1115::ADS1115(i2c_inst_t *i2c, uint sda, uint scl, uint baudrate, uint8_t address) : HighI2C(i2c, address, sda, scl, baudrate) {}

void ADS1115::init()
{
    init_i2c();
}

void ADS1115::set_gain(Pga gain)
{
    m_config.bits.pga = gain;
}
void ADS1115::set_data_rate(DataRate rate)
{
    m_config.bits.data_rate = rate;
}

void ADS1115::set_mode(Mode mode)
{
    m_config.bits.mode = mode;
}

// Reads a single-ended channel (0-3)
int16_t ADS1115::read_single_ended(uint8_t channel)
{ // channel must be a 0-3 value
    if (channel > 3)
        return -1;
    Mux mux;
    switch (channel)
    {
    case 0:
        mux = MUX_SINGLE_AIN0;
        break;
    case 1:
        mux = MUX_SINGLE_AIN1;
        break;
    case 2:
        mux = MUX_SINGLE_AIN2;
        break;
    case 3:
        mux = MUX_SINGLE_AIN3;
        break;
    default:
        return -1;
    }
    return read_differential(mux);
}

// Reads a differential channel pair
int16_t ADS1115::read_differential(Mux mux_config)
{
    // Configure the requested MUX and trigger a read
    m_config.bits.mux = mux_config;
    m_config.bits.os = OS_SINGLE_SHOT;

    // The ADS1115 expects MSB first, so we split the 16-bit config into 2 bytes
    uint8_t config_buf[2];
    config_buf[0] = (m_config.raw_value >> 8) & 0xFF; // MSB
    config_buf[1] = m_config.raw_value & 0xFF;        // LSB

    // Write configuration to the CONFIG register
    write_burst(ADS1115_REG_POINTER_CONFIG, config_buf, 2);

    // Wait for the conversion to complete
    sleep_ms(get_conversion_delay());

    // Read the 16-bit result from the CONVERT register
    uint8_t result_buf[2] = {0, 0};
    read_burst(ADS1115_REG_POINTER_CONVERT, result_buf, 2);

    // Reconstruct the 16-bit signed integer (MSB first)
    return (result_buf[0] << 8) | result_buf[1];
}

// Helper to convert ADC value to Volts
float ADS1115::compute_volts(int16_t adc_val)
{
    float fsRange;

    // Look up voltage based strictly on the struct's PGA value
    switch (m_config.bits.pga)
    {
    case PGA_6_144V:
        fsRange = 6.144f;
        break;
    case PGA_4_096V:
        fsRange = 4.096f;
        break;
    case PGA_2_048V:
        fsRange = 2.048f;
        break;
    case PGA_1_024V:
        fsRange = 1.024f;
        break;
    case PGA_0_512V:
        fsRange = 0.512f;
        break;
    case PGA_0_256V:
        fsRange = 0.256f;
        break;
    default:
        fsRange = 2.048f;
        break;
    }

    return adc_val * (fsRange / 32768.0f);
}

uint32_t ADS1115::get_conversion_delay()
{
    // Extract the data rate bits (bits 7-5) to calculate required delay
    // Formula: (1000 ms / SPS) + 1 ms margin for safety
    switch (m_config.bits.data_rate)
    {
    case RATE_8_SPS:
        return 126;
    case RATE_16_SPS:
        return 64;
    case RATE_32_SPS:
        return 33;
    case RATE_64_SPS:
        return 17;
    case RATE_128_SPS:
        return 9;
    case RATE_250_SPS:
        return 5;
    case RATE_475_SPS:
        return 4;
    case RATE_860_SPS:
        return 3;
    default:
        return 9;
    }
}

void ADS1115::set_comparator_config(CompMode mode, CompPol pol, CompLat lat, CompQue que)
{
    m_config.bits.comp_mode = mode;
    m_config.bits.comp_pol = pol;
    m_config.bits.comp_lat = lat;
    m_config.bits.comp_que = que;
}

void ADS1115::set_thresholds(int16_t lo_thresh, int16_t hi_thresh)
{
    uint8_t buf[2];

    // Format and write Low Threshold (Min)
    buf[0] = (lo_thresh >> 8) & 0xFF; // MSB
    buf[1] = lo_thresh & 0xFF;        // LSB
    write_burst(ADS1115_REG_POINTER_LO_THRESH, buf, 2);

    // Format and write High Threshold (Max)
    buf[0] = (hi_thresh >> 8) & 0xFF; // MSB
    buf[1] = hi_thresh & 0xFF;        // LSB
    write_burst(ADS1115_REG_POINTER_HI_THRESH, buf, 2);
}