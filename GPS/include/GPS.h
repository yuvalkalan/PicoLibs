#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

// GPS Struct ---
struct __attribute__((packed)) GPSData
{
    // --- 4-Byte Variables ---
    int32_t latitude;  // Degrees * 10,000,000 (+North, -South)
    int32_t longitude; // Degrees * 10,000,000 (+East, -West)

    // Pack Date/Time into exactly 32 bits (4 bytes) total
    uint32_t year : 6;   // 0-63 (Years since 2000, valid until 2063)
    uint32_t month : 4;  // 1-12
    uint32_t day : 5;    // 1-31
    uint32_t hour : 5;   // 0-23
    uint32_t minute : 6; // 0-59
    uint32_t second : 6; // 0-59

    // --- 2-Byte Variables ---
    int16_t altitude_m;   // Altitude in meters (range: -32,768m to +32,767m)
    uint16_t speed_kmh;   // km/h * 100 (e.g., 1.48 km/h is stored as 148)
    uint16_t heading_deg; // Degrees * 100 (e.g., 245.65 deg is stored as 24565)

    // --- 1-Byte Variables ---
    uint8_t satellites; // Number of satellites tracked
    bool valid;         // True if fix is active
};

class GPS
{
private:
    uart_inst_t *m_uart;
    uint m_baudrate;
    uint m_tx;
    uint m_rx;
    GPSData m_data;

    char m_buffer[128];
    int m_buffer_index;

public:
    float get_speed_kmh() const;
    uint8_t get_sat_counter() const;
    int16_t get_altitude() const;
    double get_latitude() const;
    double get_longitude() const;
    bool is_valid() const;
    float get_direction() const;
    datetime_t get_datetime() const;
    const GPSData *get_raw_value() const;

public:
    bool update();

public:
    GPS(uart_inst_t *uart, uint baudrate, uint tx, uint rx);
};
