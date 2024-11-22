#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

/* Definicje dla pinów */
#define GPIO_NODE DT_NODELABEL(gpio1)
#define SENSOR_PIN -1                   // znajdź poprawny pin
#define SENSOR_FLAGS -1                 // dodaj odpowiednie flagi (FLAGA_1 | FLAGA_2 | FLAGA_3 ...)

const struct device *gpio_dev;

int main() {
    int ret;

    /* Inicjalizacja GPIO */
    gpio_dev = DEVICE_DT_GET(GPIO_NODE);
    if (!device_is_ready(gpio_dev)) {
        printk("GPIO device not ready\n");
        return 1;
    }

    /* Konfiguracja pinu czujnika jako wejściowego */
    ret = gpio_pin_configure(gpio_dev, SENSOR_PIN, SENSOR_FLAGS);
    if (ret < 0) {
        printk("Failed to configure sensor pin\n");
        return 1;
    }

    printk("System initialized\n");

    while (1) {
        // @TODO 
        // Napisz kod odczytujący dane z czujnika ST1140
    }
    return 0;
}
