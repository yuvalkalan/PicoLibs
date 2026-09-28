#pragma once
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include "bmi2.h"
#include "bmi270_lib.h"
#ifdef __cplusplus
}
#endif

struct IMUData
{
    float acc_x, acc_y, acc_z; // Acceleration in G (sensor frame)
    float gyr_x, gyr_y, gyr_z; // Angular velocity in DPS (sensor frame)

    // Drone-frame orientation and angular rates (matching ESC sign conventions):
    // Roll  (+) = banking right
    // Pitch (+) = nose tilting forward/down
    // Yaw   (+) = rotating right (CW viewed from top)
    float roll_deg;
    float pitch_deg;
    float roll_rate_dps;
    float pitch_rate_dps;
    float yaw_rate_dps;
};

struct Bmi270I2cContext
{
    i2c_inst_t *i2c_port;
    uint8_t dev_addr;
};

class BMI270
{
public:
    BMI270(i2c_inst_t *i2c_port, uint sda_pin, uint scl_pin, uint8_t dev_addr = 0x68);

    bool init();

    /**
     * @brief Calibrates gyro biases and level accelerometer offsets.
     *        Keep the quadcopter stationary and level during calibration.
     * @param samples Number of samples to average.
     */
    void calibrate(uint16_t samples = 500);

    /**
     * @brief Reads raw sensor data and updates the complementary filter.
     * @param dt Time step in seconds since last update (if <= 0, computed automatically).
     * @return true if read succeeded.
     */
    bool update(float dt = 0.0f);

    IMUData getData() const;

private:
    i2c_inst_t *_i2c_port;
    uint _sda_pin;
    uint _scl_pin;
    uint8_t _dev_addr;

    Bmi270I2cContext _i2c_ctx;
    struct bmi2_dev _bmi;
    struct bmi2_sens_data _sensor_data;
    IMUData _data;

    // Calibration offsets
    float _gyr_offset_x, _gyr_offset_y, _gyr_offset_z;
    float _roll_acc_offset, _pitch_acc_offset;

    // Timing & Complementary filter coefficient
    uint64_t _last_update_us;
    float _alpha;

    static int8_t i2c_read_wrapper(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
    static int8_t i2c_write_wrapper(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
    static void delay_us_wrapper(uint32_t period, void *intf_ptr);
};