#pragma once
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Wrap the Bosch C headers in extern "C" to prevent linkage errors in C++
#ifdef __cplusplus
extern "C" {
#endif
#include "bmi2.h"
#include "bmi270_lib.h" 
#ifdef __cplusplus
}
#endif

// Structure to hold human-readable sensor data
struct IMUData {
    float acc_x, acc_y, acc_z; // Acceleration in G
    float gyr_x, gyr_y, gyr_z; // Angular velocity in Degrees Per Second (DPS)
};

// Context structure passed to the C-style I2C callbacks
struct Bmi270I2cContext {
    i2c_inst_t* i2c_port;
    uint8_t dev_addr;
};

class BMI270 {
public:
    /**
     * @brief Constructor for the BMI270 sensor
     * @param i2c_port Hardware I2C instance (e.g., i2c0 or i2c1)
     * @param sda_pin GPIO pin number for SDA
     * @param scl_pin GPIO pin number for SCL
     * @param dev_addr I2C device address (default is 0x68)
     */
    BMI270(i2c_inst_t* i2c_port, uint sda_pin, uint scl_pin, uint8_t dev_addr = 0x68);

    /**
     * @brief Initializes the I2C bus, loads sensor firmware, and configures max accuracy.
     * @return true if successful, false otherwise.
     */
    bool init();

    /**
     * @brief Fetches raw data from the sensor and converts it to physical units.
     * @return true if new data was successfully read, false otherwise.
     */
    bool update();

    /**
     * @brief Returns the latest converted data.
     * @return IMUData struct containing Gs and DPS values.
     */
    IMUData getData() const;

private:
    i2c_inst_t* _i2c_port;
    uint _sda_pin;
    uint _scl_pin;
    uint8_t _dev_addr;

    Bmi270I2cContext _i2c_ctx;
    struct bmi2_dev _bmi;
    struct bmi2_sens_data _sensor_data;
    IMUData _data;

    // Static wrapper functions required by the Bosch C API
    static int8_t i2c_read_wrapper(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
    static int8_t i2c_write_wrapper(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
    static void delay_us_wrapper(uint32_t period, void *intf_ptr);
};