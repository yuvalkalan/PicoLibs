#include "PID.h"
#include <cmath>

// ------------------------------------------------------------------
// PIDAxis Implementation
// ------------------------------------------------------------------
PIDAxis::PIDAxis(const PIDGains &gains)
    : _gains(gains), _integral(0.0f), _prev_measurement(0.0f),
      _d_filtered(0.0f), _first_run(true) {}

void PIDAxis::setGains(const PIDGains &gains)
{
    _gains = gains;
}

void PIDAxis::reset()
{
    _integral = 0.0f;
    _prev_measurement = 0.0f;
    _d_filtered = 0.0f;
    _first_run = true;
}

float PIDAxis::compute(float setpoint, float measurement, float dt,
                       float rate_measurement, bool use_rate_for_d)
{
    if (dt <= 0.0f)
        return 0.0f;

    float error = setpoint - measurement;

    // 1. Proportional Term
    float p_term = _gains.kp * error;

    // 2. Integral Term with Anti-Windup Clamping
    _integral += _gains.ki * error * dt;
    if (_integral > _gains.max_i)
        _integral = _gains.max_i;
    else if (_integral < -_gains.max_i)
        _integral = -_gains.max_i;

    // 3. Derivative Term (Derivative-on-Measurement to prevent setpoint kick)
    float d_raw = 0.0f;
    if (use_rate_for_d)
    {
        // In Angle Mode, gyro rate is the exact derivative of angle
        d_raw = -_gains.kd * rate_measurement;
    }
    else
    {
        if (_first_run)
        {
            _prev_measurement = measurement;
            _first_run = false;
        }
        d_raw = -_gains.kd * ((measurement - _prev_measurement) / dt);
        _prev_measurement = measurement;
    }

    // Low-pass filter on D-term to attenuate high-frequency frame vibration
    constexpr float d_lpf_alpha = 0.7f;
    _d_filtered = d_lpf_alpha * _d_filtered + (1.0f - d_lpf_alpha) * d_raw;

    // 4. Total Output & Clamping
    float output = p_term + _integral + _d_filtered;
    if (output > _gains.out_max)
        output = _gains.out_max;
    else if (output < _gains.out_min)
        output = _gains.out_min;

    return output;
}

// ------------------------------------------------------------------
// QuadPID Implementation
// ------------------------------------------------------------------
QuadPID::QuadPID(const Config &config)
    : _config(config),
      _roll_pid(config.roll_gains),
      _pitch_pid(config.pitch_gains),
      _yaw_pid(config.yaw_gains) {}

void QuadPID::reset()
{
    _roll_pid.reset();
    _pitch_pid.reset();
    _yaw_pid.reset();
}

float QuadPID::clampf(float val, float min_val, float max_val)
{
    if (val < min_val)
        return min_val;
    if (val > max_val)
        return max_val;
    return val;
}

float QuadPID::mapStickInput(int16_t raw_stick, float max_physical_val)
{
    // Clamp input to [-1000, +999]
    float clamped = clampf(static_cast<float>(raw_stick), -1000.0f, 999.0f);

    // Optional small deadband around 0 to ignore RC stick jitter
    if (std::fabs(clamped) < 10.0f)
    {
        return 0.0f;
    }

    // Normalize [-1000, +999] to [-1.0, +1.0] and scale to physical target
    float normalized = (clamped >= 0.0f) ? (clamped / 999.0f) : (clamped / 1000.0f);
    return normalized * max_physical_val;
}

void QuadPID::compute(ESC::ESCConfig &config, const IMUData &imu, float dt)
{
    ESC::ESCConfig out = {0, 0, 0, 0};

    // Clamp throttle to [0, 1999]
    if (config.throttle > 1999)
        config.throttle = 1999;
    out.throttle = config.throttle;

    // If throttle is below takeoff threshold, reset PID integrals so motors don't spool up unevenly on the ground
    if (config.throttle < _config.min_active_throttle)
    {
        reset();
        config.throttle = out.throttle;
        config.pitch = out.pitch;
        config.roll = out.roll;
        config.yaw = out.yaw;
        return;
    }

    // Map controller sticks [-1000, +999] to desired angles (deg) and yaw rate (DPS)
    float target_roll_deg = mapStickInput(config.roll, _config.max_tilt_angle_deg);
    float target_pitch_deg = mapStickInput(config.pitch, _config.max_tilt_angle_deg);
    float target_yaw_dps = mapStickInput(config.yaw, _config.max_yaw_rate_dps);

    // Compute Roll and Pitch PIDs (Self-Level Angle mode using direct gyro rate for D-term damping)
    float roll_cmd = _roll_pid.compute(target_roll_deg, imu.roll_deg, dt, imu.roll_rate_dps, true);
    float pitch_cmd = _pitch_pid.compute(target_pitch_deg, imu.pitch_deg, dt, imu.pitch_rate_dps, true);

    // Compute Yaw PID (Rate mode)
    float yaw_cmd = _yaw_pid.compute(target_yaw_dps, imu.yaw_rate_dps, dt, 0.0f, false);

    // Convert and clamp to ESC::update parameter ranges [-1000, +999]
    out.roll = static_cast<int16_t>(std::lround(clampf(roll_cmd, -1000.0f, 999.0f)));
    out.pitch = static_cast<int16_t>(std::lround(clampf(pitch_cmd, -1000.0f, 999.0f)));
    out.yaw = static_cast<int16_t>(std::lround(clampf(yaw_cmd, -1000.0f, 999.0f)));

    config.throttle = out.throttle;
    config.pitch = out.pitch;
    config.roll = out.roll;
    config.yaw = out.yaw;
    return;
}