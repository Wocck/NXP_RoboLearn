#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include "aht40.h"

int main(void) {
    const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(lpi2c1));
    float temperature, humidity;

    if (!device_is_ready(i2c_dev)) {
        printk("I2C device not ready\n");
        return 1;
    }

    if (!aht40_init(i2c_dev)) {
        printk("Failed to initialize AHT40 sensor\n");
        return 1;
    }

    while (true) {
        if (aht40_read_data(i2c_dev, &temperature, &humidity)) {
            printk("Temperature: %.2f°C, Humidity: %.2f%%\n", static_cast<double>(temperature), static_cast<double>(humidity));

        } else {
            printk("Failed to read data from AHT40 sensor\n");
        }

        k_sleep(K_SECONDS(5));
    }
    return 0;
}