#include "hcsr04.h"

HCSR04::HCSR04(const struct device* gpio_dev, uint8_t trig_pin, uint8_t echo_pin)
    : gpio_dev(gpio_dev), trig_pin(trig_pin), echo_pin(echo_pin) {}

int HCSR04::init() {
    if (!device_is_ready(gpio_dev)) {
        printk("GPIO device not ready\n");
        return -1;
    }

    if (gpio_pin_configure(gpio_dev, trig_pin, GPIO_OUTPUT) < 0) {
        printk("Failed to configure TRIG pin\n");
        return -1;
    }

    if (gpio_pin_configure(gpio_dev, echo_pin, GPIO_INPUT) < 0) {
        printk("Failed to configure ECHO pin\n");
        return -1;
    }

    printk("HC-SR04 initialized successfully\n");
    return 0;
}

int HCSR04::measureDistance() {
    uint32_t start, end;

    gpio_pin_set(gpio_dev, trig_pin, 1);
    k_busy_wait(10);  // Trigger pulse width of 10 microseconds
    gpio_pin_set(gpio_dev, trig_pin, 0);

    // Wait for ECHO signal to go HIGH
    while (gpio_pin_get(gpio_dev, echo_pin) == 0);

    start = k_cycle_get_32();

    // Wait for ECHO signal to go LOW
    while (gpio_pin_get(gpio_dev, echo_pin) == 1);

    end = k_cycle_get_32();

    // Calculate duration and convert to distance
    uint32_t duration = end - start;
    double time_s = static_cast<double>(duration) / sys_clock_hw_cycles_per_sec();
    int distance_cm = static_cast<int>((time_s * 34300) / 2); // Speed of sound = 34300 cm/s

    return distance_cm;
}
