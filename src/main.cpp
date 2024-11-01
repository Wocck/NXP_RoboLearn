#include <iostream>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS 1000  // 1000 ms = 1 second

#define LED0_NODE DT_ALIAS(led0)
static const gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// Global variables to store execution times
uint32_t cout_duration = 0;
uint32_t printf_duration = 0;

void measure_time_cout() {
    uint32_t start_time = k_cycle_get_32();
    std::cout << "Testing std::cout timing in Zephyr" << std::endl;
    uint32_t end_time = k_cycle_get_32();
    cout_duration = k_cyc_to_ns_floor32(end_time - start_time) / 1000;  // Convert to microseconds
}

void measure_time_printf() {
    uint32_t start_time = k_cycle_get_32();
    printf("Testing printf timing in Zephyr\n");
    uint32_t end_time = k_cycle_get_32();
    printf_duration = k_cyc_to_ns_floor32(end_time - start_time) / 1000;  // Convert to microseconds
}

int main() {
    int ret;

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

        // Measure execution time of std::cout and printf
        measure_time_cout();
        measure_time_printf();

        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}
