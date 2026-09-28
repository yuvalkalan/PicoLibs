#include "BMI270.h"
#include <cmath>

static constexpr float RAD_TO_DEG = 57.295779513f;

BMI270::BMI270(i2c_inst_t *i2c_port, uint sda_pin, uint scl_pin, uint8_t dev_addr)
    : _i2c_port(i2c_port), _sda_pin(sda_pin), _scl_pin(scl_pin), _dev_addr(dev_addr),
      _gyr_offset_x(0.0f), _gyr_offset_y(0.0f), _gyr_offset_z(0.0f),
      _roll_acc_offset(0.0f), _pitch_acc_offset(0.0f),
      _last_update_us(0), _alpha(0.985f)
{
    _data = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    _sensor_data = {{0}};
}

bool BMI270::init()
{
    // 1. Setup Pico Hardware I2C at 400 kHz
    i2c_init(_i2c_port, 400 * 1000);
    gpio_set_function(_sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(_scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(_sda_pin);
    gpio_pull_up(_scl_pin);

    // 2. Prepare context for C-callbacks
    _i2c_ctx.i2c_port = _i2c_port;
    _i2c_ctx.dev_addr = _dev_addr;

    // 3. Link Bosch device structure
    _bmi.intf = BMI2_I2C_INTF;
    _bmi.intf_ptr = &_i2c_ctx;
    _bmi.read = i2c_read_wrapper;
    _bmi.write = i2c_write_wrapper;
    _bmi.delay_us = delay_us_wrapper;
    _bmi.read_write_len = 32;
    _bmi.config_file_ptr = nullptr;

    // 4. Initialize sensor
    int8_t rslt = bmi270_init(&_bmi);
    if (rslt != BMI2_OK)
        return false;

    // Disable Advance Power Save (APS) to eliminate the 450us read penalty
    rslt = bmi2_set_adv_power_save(BMI2_DISABLE, &_bmi);
    if (rslt != BMI2_OK)
        return false;

    // 5. Configure for Flight Control (400Hz ODR, +/-8G Accel, +/-1000 DPS Gyro)
    struct bmi2_sens_config config[2];

    config[0].type = BMI2_ACCEL;
    config[0].cfg.acc.odr = BMI2_ACC_ODR_400HZ;
    config[0].cfg.acc.range = BMI2_ACC_RANGE_8G; // 4096 LSB/G
    config[0].cfg.acc.bwp = BMI2_ACC_OSR2_AVG2;  // Low latency hardware LPF
    config[0].cfg.acc.filter_perf = BMI2_PERF_OPT_MODE;

    config[1].type = BMI2_GYRO;
    config[1].cfg.gyr.odr = BMI2_GYR_ODR_400HZ;
    config[1].cfg.gyr.range = BMI2_GYR_RANGE_1000; // 32.8 LSB/DPS
    config[1].cfg.gyr.bwp = BMI2_GYR_OSR2_MODE;
    config[1].cfg.gyr.noise_perf = BMI2_PERF_OPT_MODE;
    config[1].cfg.gyr.filter_perf = BMI2_PERF_OPT_MODE;

    rslt = bmi2_set_sensor_config(config, 2, &_bmi);
    if (rslt != BMI2_OK)
        return false;

    // 6. Enable Accel and Gyro
    uint8_t sens_list[2] = {BMI2_ACCEL, BMI2_GYRO};
    rslt = bmi2_sensor_enable(sens_list, 2, &_bmi);

    _last_update_us = time_us_64();
    return (rslt == BMI2_OK);
}

void BMI270::calibrate(uint16_t samples)
{
    float gx_sum = 0.0f, gy_sum = 0.0f, gz_sum = 0.0f;
    float roll_sum = 0.0f, pitch_sum = 0.0f;
    uint16_t valid = 0;

    for (uint16_t i = 0; i < samples; i++)
    {
        if (bmi2_get_sensor_data(&_sensor_data, &_bmi) == BMI2_OK)
        {
            float ax = _sensor_data.acc.x / 4096.0f;
            float ay = _sensor_data.acc.y / 4096.0f;
            float az = _sensor_data.acc.z / 4096.0f;

            gx_sum += _sensor_data.gyr.x / 32.8f;
            gy_sum += _sensor_data.gyr.y / 32.8f;
            gz_sum += _sensor_data.gyr.z / 32.8f;

            // Tilt right -> acc_x < 0 -> positive roll
            roll_sum += std::atan2(-ax, std::sqrt(ay * ay + az * az)) * RAD_TO_DEG;
            // Tilt forward -> acc_y < 0 -> positive pitch
            pitch_sum += std::atan2(-ay, std::sqrt(ax * ax + az * az)) * RAD_TO_DEG;
            valid++;
        }
        sleep_ms(2);
    }

    if (valid > 0)
    {
        _gyr_offset_x = gx_sum / valid;
        _gyr_offset_y = gy_sum / valid;
        _gyr_offset_z = gz_sum / valid;
        _roll_acc_offset = roll_sum / valid;
        _pitch_acc_offset = pitch_sum / valid;
    }
    _data.roll_deg = 0.0f;
    _data.pitch_deg = 0.0f;
    _last_update_us = time_us_64();
}

bool BMI270::update(float dt)
{
    int8_t rslt = bmi2_get_sensor_data(&_sensor_data, &_bmi);
    if (rslt != BMI2_OK)
        return false;

    uint64_t now_us = time_us_64();
    if (dt <= 0.0f)
    {
        dt = (now_us - _last_update_us) * 1e-6f;
    }
    _last_update_us = now_us;
    if (dt <= 0.0f || dt > 0.5f)
        dt = 0.0025f; // Fallback to 400Hz if timer wraps or stalls

    // 1. Convert raw LSB to Gs (+/- 8G -> 4096 LSB/G)
    _data.acc_x = _sensor_data.acc.x / 4096.0f;
    _data.acc_y = _sensor_data.acc.y / 4096.0f;
    _data.acc_z = _sensor_data.acc.z / 4096.0f;

    // 2. Convert raw LSB to DPS (+/- 1000 DPS -> 32.8 LSB/DPS) and remove bias
    _data.gyr_x = (_sensor_data.gyr.x / 32.8f) - _gyr_offset_x;
    _data.gyr_y = (_sensor_data.gyr.y / 32.8f) - _gyr_offset_y;
    _data.gyr_z = (_sensor_data.gyr.z / 32.8f) - _gyr_offset_z;

    // 3. Map sensor axes to Drone Flight Axes (matching ESC::update conventions)
    // +X = Right, +Y = Forward, +Z = Up
    _data.roll_rate_dps = _data.gyr_y;   // Rolling right  = +gyr_y
    _data.pitch_rate_dps = -_data.gyr_x; // Pitching fwd   = -gyr_x
    _data.yaw_rate_dps = -_data.gyr_z;   // Yawing right   = -gyr_z

    // 4. Calculate Accelerometer Tilt Angles
    // Tilting right -> acc_x < 0 -> roll_acc > 0
    float roll_acc = (std::atan2(-_data.acc_x, std::sqrt(_data.acc_y * _data.acc_y + _data.acc_z * _data.acc_z)) * RAD_TO_DEG) - _roll_acc_offset;
    // Tilting forward -> acc_y < 0 -> pitch_acc > 0
    float pitch_acc = (std::atan2(-_data.acc_y, std::sqrt(_data.acc_x * _data.acc_x + _data.acc_z * _data.acc_z)) * RAD_TO_DEG) - _pitch_acc_offset;

    // 5. Complementary Filter (combines fast gyro integration with drift-free accel angle)
    _data.roll_deg = _alpha * (_data.roll_deg + _data.roll_rate_dps * dt) + (1.0f - _alpha) * roll_acc;
    _data.pitch_deg = _alpha * (_data.pitch_deg + _data.pitch_rate_dps * dt) + (1.0f - _alpha) * pitch_acc;

    return true;
}

IMUData BMI270::getData() const
{
    return _data;
}

int8_t BMI270::i2c_read_wrapper(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    Bmi270I2cContext *ctx = static_cast<Bmi270I2cContext *>(intf_ptr);
    i2c_write_blocking(ctx->i2c_port, ctx->dev_addr, &reg_addr, 1, true);
    int bytes_read = i2c_read_blocking(ctx->i2c_port, ctx->dev_addr, reg_data, len, false);
    return (bytes_read > 0) ? BMI2_OK : BMI2_E_COM_FAIL;
}

int8_t BMI270::i2c_write_wrapper(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    Bmi270I2cContext *ctx = static_cast<Bmi270I2cContext *>(intf_ptr);
    uint8_t buf[len + 1];
    buf[0] = reg_addr;
    for (uint32_t i = 0; i < len; i++)
    {
        buf[i + 1] = reg_data[i];
    }
    int bytes_written = i2c_write_blocking(ctx->i2c_port, ctx->dev_addr, buf, len + 1, false);
    return (bytes_written > 0) ? BMI2_OK : BMI2_E_COM_FAIL;
}

void BMI270::delay_us_wrapper(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;
    sleep_us(period);
}