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
    struct ESCConfig
    {
        uint16_t throttle; // value between 0 to 199
        int16_t roll;      // value between -999 to 1000
        int16_t pitch;     // value between -999 to 1000
        int16_t yaw;       // value between -999 to 1000
    };

private:
    DShot600 m_motors[4];

public:
    ESC(const MotorsConfig &config);
    void init();
    void update(const ESCConfig &config);
};
