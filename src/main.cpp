#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED_PIN 2 // Replace with the correct pin for your board if needed
#define LED_PORT "GPIO1"

int main() {
    const struct device *led_dev = device_get_binding(LED_PORT);

    if (led_dev == nullptr) {
        printk("Failed to bind to LED device\n");
        return 1;
    }

    gpio_pin_configure(led_dev, LED_PIN, GPIO_OUTPUT);

    while (true) {
        gpio_pin_toggle(led_dev, LED_PIN);
        printk("LED toggled!\n");
        k_msleep(500);  // Wait 500 ms
    }
    return 0;
}
