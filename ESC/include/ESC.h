#pragma once
#include "Dshot600.h"

/**
 * Motors location:
 *   1. Rear Left (CCW)
 *   2. Front Left (CW)
 *   3. Front Right (CCW)
 *   4. Rear Right (CW)
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
private:
    DShot600 m_motors[4];
    uint16_t m_throttle;
    int16_t m_roll, m_pitch, m_yaw;

public:
    ESC(const uint8_t (&pins)[4]);
    void init();
    void update(uint16_t throttle, int16_t roll, int16_t pitch, int16_t yaw);
};
