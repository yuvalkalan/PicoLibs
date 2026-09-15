#pragma once
#include "Configuration.h"
#include "pico/stdlib.h"

// Initializes the DShot PIO state machine on a specific pin
void dshot_init(uint pin, uint sm);

// Sends a 16-bit DShot frame to the specified state machine
void dshot_send_frame(uint sm, uint16_t frame);