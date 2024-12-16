#ifndef HCSR04_H
#define HCSR04_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

class HCSR04 {
public:
    HCSR04(const struct device* gpio_dev, uint8_t trig_pin, uint8_t echo_pin);

    int init();
    int measureDistance();

private:
    const struct device* gpio_dev;
    uint8_t trig_pin;
    uint8_t echo_pin;

    static constexpr uint32_t MIN_DISTANCE_CM = 10;          // Minimal distance to activate buzzer
    static constexpr uint32_t MAX_DISTANCE_CM = 20;          // Max distance for intermittent buzzer
    static constexpr uint32_t ZERO_DISTANCE = 2000;          // Zero distance threshold
};

#endif // HCSR04_H
