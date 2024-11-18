#ifndef AHT40_H
#define AHT40_H

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#define AHT40_I2C_ADDRESS 0x38

bool aht40_init(const struct device *i2c_dev);
bool aht40_read_data(const struct device *i2c_dev, float *temperature, float *humidity);

#endif // AHT40_H