#pragma once

#include "hardware/gpio.h"
#include <pico/time.h>
#include "pico/mutex.h"

#define LED_BLINK_SLOW 1000
#define LED_BLINK_FAST 500

class LedControl
{
private:
    uint32_t blink_speed_ms;
    bool is_on;
    volatile absolute_time_t last_blink_time;

private:
    void toggle();

public:
    LedControl();
    void update();
    void set_blink_speed(uint32_t speed_ms);
};