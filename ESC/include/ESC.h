#pragma once
#include "DShot600.h"

/**
 * Motors location:
 *   1. Rear Left (CCW)
 *   2. Front Left (CW)
 *   3. Rear Right (CW)
 *   4. Front Right (CCW)
 * Calculates the motor mixing for a custom quadcopter layout.
 *
 * Sign Conventions used:
 * - Throttle: Positive increases overall speed.
 * - Roll:     Positive banks right (Left motors speed up, Right motors slow down).
 * - Pitch:    Positive pitches nose down (Rear motors speed up, Front motors slow down).
 * - Yaw:      Positive yaws right (CCW motors speed up, CW motors slow down).
 */

class ESC
{
public:
    struct MotorsConfig
    {
        uint8_t pin1;
        uint8_t pin2;
        uint8_t pin3;
        uint8_t pin4;
        PIO pio;
    };

private:
    DShot600 m_motors[4];
    uint16_t m_throttle;
    int16_t m_roll, m_pitch, m_yaw;

public:
    ESC(const MotorsConfig &config);
    void init();
    void update(uint16_t throttle, int16_t roll, int16_t pitch, int16_t yaw);
};
