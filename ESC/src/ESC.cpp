#include "ESC.h"

// TODO: change hardcoded pios, sm and dma channels
ESC::ESC(const MotorsConfig &config) : m_motors{
                                           DShot600(config.pio, 0, config.pin1),
                                           DShot600(config.pio, 1, config.pin2),
                                           DShot600(config.pio, 2, config.pin3),
                                           DShot600(config.pio, 3, config.pin4)},
                                       m_throttle(0), m_roll(0), m_pitch(0), m_yaw(0)
{
}

void ESC::init()
{
    for (size_t i = 0; i < 4; i++)
    {
        m_motors[i].init();
    }
    for (size_t i = 0; i < 4; i++)
    {
        m_motors[i].set_throttle(0);
        m_motors[i].start();
    }
    sleep_ms(2000); // Wait 2 second for ESC to initialize and beep
}

void ESC::update(uint16_t throttle, int16_t roll, int16_t pitch, int16_t yaw)
{
    m_throttle = throttle;
    m_roll = roll;
    m_pitch = pitch;
    m_yaw = yaw;
    // Motor 1: Rear Left (CCW)
    // To pitch fwd: speed up (+). To roll right: speed up (+). To yaw right: speed up (+).
    int16_t raw_m1 = throttle + pitch + roll + yaw;

    // Motor 2: Front Left (CW)
    // To pitch fwd: slow down (-). To roll right: speed up (+). To yaw right: slow down (-).
    int16_t raw_m2 = throttle - pitch + roll - yaw;

    // Motor 3: Front Right (CCW)
    // To pitch fwd: slow down (-). To roll right: slow down (-). To yaw right: speed up (+).
    int16_t raw_m3 = throttle + pitch - roll - yaw;

    // Motor 4: Rear Right (CW)
    // To pitch fwd: speed up (+). To roll right: slow down (-). To yaw right: slow down (-).
    int16_t raw_m4 = throttle - pitch - roll + yaw;

    int16_t raw_values[4] = {raw_m1, raw_m2, raw_m3, raw_m4};
    // Clamp values to ensure they stay within valid ESC signal ranges
    for (size_t i = 0; i < 4; i++)
    {
        // clamp value
        int16_t value = raw_values[i];
        if (value < DShot600::Cmd::MIN_THROTTLE)
            value = DShot600::Cmd::MIN_THROTTLE;
        else if (value > DShot600::Cmd::MAX_THROTTLE)
            value = DShot600::Cmd::MAX_THROTTLE;

        m_motors[i].set_throttle(value);
    }
}