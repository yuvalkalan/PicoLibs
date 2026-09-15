#pragma once

#include "pico/stdlib.h"
#include "HighI2C.h"

// I2C Addresses (Depending on ADDR pin connection)
#define ADS1115_ADDRESS_GND 0x48
#define ADS1115_ADDRESS_VDD 0x49
#define ADS1115_ADDRESS_SDA 0x4A
#define ADS1115_ADDRESS_SCL 0x4B

#define ADS1115_REG_POINTER_CONVERT 0x00
#define ADS1115_REG_POINTER_CONFIG 0x01

class ADS1115 : public HighI2C
{
public:
    // Bit 15: Operational status / Single-shot conversion start
    enum OsStatus
    {
        OS_NO_EFFECT = 0,  // Write: No effect / Read: Currently performing conversion
        OS_SINGLE_SHOT = 1 // Write: Start single conversion / Read: Not performing conversion
    };
    // Bits 14-12: Input multiplexer configuration
    enum Mux
    {
        MUX_DIFF_AIN0_AIN1 = 0,
        MUX_DIFF_AIN0_AIN3 = 1,
        MUX_DIFF_AIN1_AIN3 = 2,
        MUX_DIFF_AIN2_AIN3 = 3,
        MUX_SINGLE_AIN0 = 4,
        MUX_SINGLE_AIN1 = 5,
        MUX_SINGLE_AIN2 = 6,
        MUX_SINGLE_AIN3 = 7
    };
    // Bits 11-9: Programmable gain amplifier configuration
    enum Pga
    {
        PGA_6_144V = 0, // +/- 6.144V
        PGA_4_096V = 1, // +/- 4.096V
        PGA_2_048V = 2, // +/- 2.048V (Default)
        PGA_1_024V = 3, // +/- 1.024V
        PGA_0_512V = 4, // +/- 0.512V
        PGA_0_256V = 5  // +/- 0.256V
    };
    // Bit 8: Device operating mode
    enum Mode
    {
        MODE_CONTINUOUS = 0, // Continuous-conversion mode
        MODE_SINGLE_SHOT = 1 // Power-down single-shot mode (Default)
    };
    // Bits 7-5: Data rate
    enum DataRate
    {
        RATE_8_SPS = 0,
        RATE_16_SPS = 1,
        RATE_32_SPS = 2,
        RATE_64_SPS = 3,
        RATE_128_SPS = 4, // (Default)
        RATE_250_SPS = 5,
        RATE_475_SPS = 6,
        RATE_860_SPS = 7
    };
    // Bit 4: Comparator mode
    enum CompMode
    {
        COMP_MODE_TRADITIONAL = 0, // Traditional comparator (Default)
        COMP_MODE_WINDOW = 1       // Window comparator
    };
    // Bit 3: Comparator polarity
    enum CompPol
    {
        COMP_POL_ACTIVE_LOW = 0, // Active low (Default)
        COMP_POL_ACTIVE_HIGH = 1 // Active high
    };
    // Bit 2: Latching comparator
    enum CompLat
    {
        COMP_LAT_NON_LATCHING = 0, // Non-latching comparator (Default)
        COMP_LAT_LATCHING = 1      // Latching comparator
    };
    // Bits 1-0: Comparator queue and disable
    enum CompQue
    {
        COMP_QUE_1_CONV = 0, // Assert after one conversion
        COMP_QUE_2_CONV = 1, // Assert after two conversions
        COMP_QUE_4_CONV = 2, // Assert after four conversions
        COMP_QUE_DISABLE = 3 // Disable comparator (Default)
    };
    // The Configuration Union
    union ConfigRegister
    {
        struct
        {
            uint16_t comp_que : 2;  // Bits 0-1
            uint16_t comp_lat : 1;  // Bit 2
            uint16_t comp_pol : 1;  // Bit 3
            uint16_t comp_mode : 1; // Bit 4
            uint16_t data_rate : 3; // Bits 5-7
            uint16_t mode : 1;      // Bit 8
            uint16_t pga : 3;       // Bits 9-11
            uint16_t mux : 3;       // Bits 12-14
            uint16_t os : 1;        // Bit 15
        } bits;

        uint16_t raw_value;

        // Default constructor sets the default values upon creation
        ConfigRegister()
        {
            raw_value = 0; // Initialize memory to 0
            bits.comp_que = COMP_QUE_DISABLE;
            bits.comp_lat = COMP_LAT_NON_LATCHING;
            bits.comp_pol = COMP_POL_ACTIVE_LOW;
            bits.comp_mode = COMP_MODE_TRADITIONAL;
            bits.data_rate = RATE_128_SPS;
            bits.mode = MODE_SINGLE_SHOT;
            bits.pga = PGA_2_048V;
            bits.mux = MUX_DIFF_AIN0_AIN1;
            bits.os = OS_NO_EFFECT;
        }
    };

private:
    ConfigRegister m_config;

public:
    ADS1115(i2c_inst_t *i2c, uint sda, uint scl, uint baudrate, uint8_t address = ADS1115_ADDRESS_GND);

    void init();
    void set_gain(Pga gain);
    void set_data_rate(DataRate rate);
    void set_mode(Mode mode);

    // Reads a single-ended channel (0-3)
    int16_t read_single_ended(uint8_t channel);

    // Reads a differential channel pair
    int16_t read_differential(Mux mux_config);

    // Helper to convert ADC value to Volts
    float compute_volts(int16_t adc_val);

private:
    // Gets the delay time in ms needed for the current data rate
    uint32_t get_conversion_delay();
};
