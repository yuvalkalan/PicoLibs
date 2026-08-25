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
    int32_t latitude; 
    int32_t longitude; 

    uint32_t year : 6;   
    uint32_t month : 4;  
    uint32_t day : 5;    
    uint32_t hour : 5;   
    uint32_t minute : 6; 
    uint32_t second : 6; 

    int16_t altitude_m;   
    uint16_t speed_kmh;   
    uint16_t heading_deg; 

    uint8_t satellites; 
    bool valid;         
};

class GPS
{
private:
    uart_inst_t *m_uart;
    uint m_baudrate;
    uint m_tx;
    uint m_rx;
    GPSData m_data;

    static GPS* s_instance;
    static void on_uart_rx_isr();
    void isr_handler();

    // --- ISR Temporary Buffer ---
    char m_isr_buffer[128];
    int m_isr_index;

    // --- Mailbox Buffers ---
    volatile char m_latest_rmc[128];
    volatile char m_latest_gga[128];
    volatile bool m_rmc_ready;
    volatile bool m_gga_ready;
    
    // Main loop parser buffer
    char m_buffer[128]; 

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
    ~GPS();
};