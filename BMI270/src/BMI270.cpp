#include "BMI270.h"

// ------------------------------------------------------------------
// Constructor
// ------------------------------------------------------------------
BMI270::BMI270(i2c_inst_t* i2c_port, uint sda_pin, uint scl_pin, uint8_t dev_addr)
    : _i2c_port(i2c_port), _sda_pin(sda_pin), _scl_pin(scl_pin), _dev_addr(dev_addr) {
    
    // Initialize internal data structures to zero
    _data = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    _sensor_data = {{0}};
}

// ------------------------------------------------------------------
// Public Methods
// ------------------------------------------------------------------
bool BMI270::init() {
    // 1. Setup Pico Hardware I2C
    i2c_init(_i2c_port, 400 * 1000); // 400 kHz Fast Mode
    gpio_set_function(_sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(_scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(_sda_pin);
    gpio_pull_up(_scl_pin);

    // 2. Prepare context for C-callbacks
    _i2c_ctx.i2c_port = _i2c_port;
    _i2c_ctx.dev_addr = _dev_addr;

    // 3. Link the Bosch device structure to our C++ static wrappers
    _bmi.intf = BMI2_I2C_INTF;
    _bmi.intf_ptr = &_i2c_ctx;
    _bmi.read = i2c_read_wrapper;
    _bmi.write = i2c_write_wrapper;
    _bmi.delay_us = delay_us_wrapper;
    _bmi.read_write_len = 32;
    _bmi.config_file_ptr = nullptr;

    // 4. Initialize sensor (Uploads firmware automatically)
    int8_t rslt = bmi270_init(&_bmi);
    if (rslt != BMI2_OK) {
        return false;
    }

    // 5. Configure for MAXIMUM ACCURACY 
    struct bmi2_sens_config config[2];

    // Accelerometer: 100Hz, +/- 2G Range, Average of 64 samples
    config[0].type = BMI2_ACCEL;
    config[0].cfg.acc.odr = BMI2_ACC_ODR_100HZ;
    config[0].cfg.acc.range = BMI2_ACC_RANGE_2G;
    config[0].cfg.acc.bwp = BMI2_ACC_RES_AVG64;
    config[0].cfg.acc.filter_perf = BMI2_PERF_OPT_MODE;

    // Gyroscope: 100Hz, +/- 125 Degrees Per Second (Highest Resolution)
    config[1].type = BMI2_GYRO;
    config[1].cfg.gyr.odr = BMI2_GYR_ODR_100HZ;
    config[1].cfg.gyr.range = BMI2_GYR_RANGE_125;
    config[1].cfg.gyr.bwp = BMI2_GYR_NORMAL_MODE;
    config[1].cfg.gyr.noise_perf = BMI2_PERF_OPT_MODE;
    config[1].cfg.gyr.filter_perf = BMI2_PERF_OPT_MODE;

    rslt = bmi2_set_sensor_config(config, 2, &_bmi);
    if (rslt != BMI2_OK) return false;

    // 6. Enable the sensors
    uint8_t sens_list[2] = {BMI2_ACCEL, BMI2_GYRO};
    rslt = bmi2_sensor_enable(sens_list, 2, &_bmi);
    
    return (rslt == BMI2_OK);
}

bool BMI270::update() {
    // Read raw data from the sensor
    int8_t rslt = bmi2_get_sensor_data(&_sensor_data, &_bmi);
    
    if (rslt == BMI2_OK) {
        // Convert raw LSB to G's (Range +/- 2G -> 16384 LSB/G)
        _data.acc_x = _sensor_data.acc.x / 16384.0f;
        _data.acc_y = _sensor_data.acc.y / 16384.0f;
        _data.acc_z = _sensor_data.acc.z / 16384.0f;

        // Convert raw LSB to DPS (Range +/- 125 DPS -> 262.4 LSB/DPS)
        _data.gyr_x = _sensor_data.gyr.x / 262.4f;
        _data.gyr_y = _sensor_data.gyr.y / 262.4f;
        _data.gyr_z = _sensor_data.gyr.z / 262.4f;
        
        return true;
    }
    return false;
}

IMUData BMI270::getData() const {
    return _data;
}

// ------------------------------------------------------------------
// Private Static C Callbacks
// ------------------------------------------------------------------
int8_t BMI270::i2c_read_wrapper(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    Bmi270I2cContext* ctx = static_cast<Bmi270I2cContext*>(intf_ptr);
    
    i2c_write_blocking(ctx->i2c_port, ctx->dev_addr, &reg_addr, 1, true);
    int bytes_read = i2c_read_blocking(ctx->i2c_port, ctx->dev_addr, reg_data, len, false);
    
    return (bytes_read > 0) ? BMI2_OK : BMI2_E_COM_FAIL;
}

int8_t BMI270::i2c_write_wrapper(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
    Bmi270I2cContext* ctx = static_cast<Bmi270I2cContext*>(intf_ptr);
    
    // Pico SDK requires register address and data in a single buffer
    uint8_t buf[len + 1];
    buf[0] = reg_addr;
    for (uint32_t i = 0; i < len; i++) {
        buf[i + 1] = reg_data[i];
    }
    
    int bytes_written = i2c_write_blocking(ctx->i2c_port, ctx->dev_addr, buf, len + 1, false);
    return (bytes_written > 0) ? BMI2_OK : BMI2_E_COM_FAIL;
}

void BMI270::delay_us_wrapper(uint32_t period, void *intf_ptr) {
    // intf_ptr is unused for the delay function
    (void)intf_ptr; 
    sleep_us(period);
}