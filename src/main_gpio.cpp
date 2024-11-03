#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

/* Definicje dla D15 */
#define D15_GPIO_NODE DT_NODELABEL(gpio1)
#define D15_PIN 27

/* Opcjonalnie, jeśli potrzebujesz flag, możesz je zdefiniować */
#define D15_FLAGS GPIO_ACTIVE_LOW

int main(void)
{
    const struct device *gpio_dev;
    int ret;

    /* Pobierz urządzenie GPIO */
    gpio_dev = DEVICE_DT_GET(D15_GPIO_NODE);
    if (!device_is_ready(gpio_dev)) {
        printk("GPIO device not ready\n");
        return 1;
    }

    /* Skonfiguruj pin D15 jako wyjście */
    ret = gpio_pin_configure(gpio_dev, D15_PIN, GPIO_OUTPUT_ACTIVE | D15_FLAGS);
    if (ret < 0) {
        printk("Failed to configure GPIO pin\n");
        return 1;
    }

    while (1) {
        gpio_pin_toggle(gpio_dev, D15_PIN);
        k_msleep(500);
    }
    return 0;
}
