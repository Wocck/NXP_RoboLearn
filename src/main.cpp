#include <iostream>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS 1000  // 1000 ms = 1 second

#define LED0_NODE DT_ALIAS(led0)

// Ensure LED0 alias is defined in the device tree
static const gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main() {
    int ret;
    bool led_state = true;

    if (!gpio_is_ready_dt(&led)) {
        std::cout << "Error: LED device not ready" << std::endl;
        return -1;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        std::cout << "Error: Failed to configure LED pin" << std::endl;
        return -1;
    }

    while (true) {
        ret = gpio_pin_toggle_dt(&led);
        if (ret < 0) {
            std::cout << "Error: Failed to toggle LED" << std::endl;
            return -1;
        }

        led_state = !led_state;
        std::cout << "LED state: " << (led_state ? "ON" : "OFF") << std::endl;
        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}
