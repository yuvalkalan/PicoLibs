#pragma once
#include <stdint.h>
#include "BMI270.h"
#include "ESC.h"

struct PIDGains
{
    float kp;
    float ki;
    float kd;
    float max_i;   // Maximum integral term magnitude (anti-windup)
    float out_min; // Minimum PID output (-1000)
    float out_max; // Maximum PID output (+999)
};

class PIDAxis
{
public:
    PIDAxis(const PIDGains &gains);

    void setGains(const PIDGains &gains);
    void reset();

    /**
     * @brief Computes PID output using Setpoint and Measurement.
     * @param setpoint Desired value (e.g. target angle in deg or target rate in DPS)
     * @param measurement Current value from IMU
     * @param dt Time step in seconds
     * @param rate_measurement Optional direct gyro rate (DPS) for the D-term in Angle mode
     * @param use_rate_for_d If true, uses -rate_measurement*kd instead of numerical derivative
     */
    float compute(float setpoint, float measurement, float dt,
                  float rate_measurement = 0.0f, bool use_rate_for_d = false);

private:
    PIDGains _gains;
    float _integral;
    float _prev_measurement;
    float _d_filtered;
    bool _first_run;
};

class QuadPID
{
public:
    struct Config
    {
        PIDGains roll_gains;
        PIDGains pitch_gains;
        PIDGains yaw_gains;
        float max_tilt_angle_deg;     // Max roll/pitch angle at full stick deflection (e.g. 30.0 deg)
        float max_yaw_rate_dps;       // Max yaw rate at full stick deflection (e.g. 200.0 DPS)
        uint16_t min_active_throttle; // Throttle threshold to enable integral accumulation
    };

    QuadPID(const Config &config);

    void reset();

    /**
     * @brief Calculates the stabilized Roll, Pitch, Yaw, and Throttle values for ESC::update().
     * @param rc_throttle Controller throttle [0, 1999]
     * @param rc_roll     Controller roll     [-1000, 999]
     * @param rc_pitch    Controller pitch    [-1000, 999]
     * @param rc_yaw      Controller yaw      [-1000, 999]
     * @param imu         Latest IMUData from BMI270
     * @param dt          Loop time step in seconds
     * @return FlightControlOutput ready to pass to ESC::update()
     */
    void compute(ESC::ESCConfig &config, const IMUData &imu, float dt);

private:
    Config _config;
    PIDAxis _roll_pid;
    PIDAxis _pitch_pid;
    PIDAxis _yaw_pid;

    static float clampf(float val, float min_val, float max_val);
    static float mapStickInput(int16_t raw_stick, float max_physical_val);
};