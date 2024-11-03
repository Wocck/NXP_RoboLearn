#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

/* Definitions for D15 and D14 */
#define GPIO_NODE DT_NODELABEL(gpio1)
#define D15_PIN 16  // LED pin
#define D14_PIN 17  // Button pin

#define D15_FLAGS (GPIO_OUTPUT | GPIO_ACTIVE_LOW)  // LED starts OFF
#define D14_FLAGS (GPIO_INPUT | GPIO_PULL_UP | GPIO_ACTIVE_LOW)  // Button with pull-up

#define DEBOUNCE_DELAY_MS 50

int main()
{
    const struct device *gpio_dev;
    int ret;
    int button_state, last_button_state = 1;
    bool led_state = false;

    /* Get GPIO device */
    gpio_dev = DEVICE_DT_GET(GPIO_NODE);
    if (!device_is_ready(gpio_dev)) {
        printk("GPIO device not ready\n");
        return 1;
    }

    /* Configure pin D15 as output for LED */
    ret = gpio_pin_configure(gpio_dev, D15_PIN, D15_FLAGS);
    if (ret < 0) {
        printk("Failed to configure D15 pin\n");
        return 1;
    }

    /* Configure pin D14 as input with pull-up */
    ret = gpio_pin_configure(gpio_dev, D14_PIN, D14_FLAGS);
    if (ret < 0) {
        printk("Failed to configure D14 pin\n");
        return 1;
    }

    printk("System initialized\n");

    while (1) {
        button_state = gpio_pin_get(gpio_dev, D14_PIN);

        if (button_state != last_button_state) {
            k_msleep(DEBOUNCE_DELAY_MS);

            button_state = gpio_pin_get(gpio_dev, D14_PIN);
            if (button_state == 0) {
                led_state = !led_state;
                gpio_pin_set(gpio_dev, D15_PIN, led_state ? 0 : 1);
            }
        }

        last_button_state = button_state;
        k_msleep(10);
    }
    return 0;
}
