#include "aht40.h"
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(AHT40, LOG_LEVEL_DBG);

bool aht40_init(const struct device *i2c_dev) {
    uint8_t init_cmd[3] = {0xBE, 0x08, 0x00};
    int ret = i2c_write(i2c_dev, init_cmd, sizeof(init_cmd), AHT40_I2C_ADDRESS);
    if (ret < 0) {
        LOG_ERR("Failed to initialize AHT40: %d", ret);
        return false;
    }
    k_msleep(10);  // Allow time for sensor to initialize
    return true;
}

bool aht40_read_data(const struct device *i2c_dev, float *temperature, float *humidity) {
    uint8_t trigger_cmd[3] = {0xAC, 0x33, 0x00};
    uint8_t data[6];
    int ret;

    // Trigger measurement
    ret = i2c_write(i2c_dev, trigger_cmd, sizeof(trigger_cmd), AHT40_I2C_ADDRESS);
    if (ret < 0) {
        LOG_ERR("Failed to trigger AHT40 measurement: %d", ret);
        return false;
    }

    k_msleep(80);  // Wait for measurement to complete

    // Read the measurement data
    ret = i2c_read(i2c_dev, data, 6, AHT40_I2C_ADDRESS);
    if (ret < 0) {
        LOG_ERR("Failed to read data from AHT40: %d", ret);
        return false;
    }

    // Process the raw data to extract temperature and humidity
    uint32_t humidity_raw = ((data[1] << 12) | (data[2] << 4) | (data[3] >> 4));
    uint32_t temperature_raw = ((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5];

    *humidity = ((float)humidity_raw / 1048576) * 100;
    *temperature = ((float)temperature_raw / 1048576) * 200 - 50;

    return true;
}
