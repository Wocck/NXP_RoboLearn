#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

/* Definicje dla D15 i D14 */
#define GPIO_NODE DT_NODELABEL(gpio1)
#define D15_PIN 16
#define D14_PIN 17

#define D15_FLAGS (GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_LOW)
#define D14_FLAGS (GPIO_INPUT | GPIO_PULL_UP | GPIO_ACTIVE_LOW)

#define DEBOUNCE_DELAY_MS 50

static const struct device *gpio_dev;
static struct gpio_callback button_cb_data;
static struct k_work_delayable button_work;

/* Handler pracy opóźnionej do obsługi debouncingu */
static void button_pressed_work_handler(struct k_work *work)
{
    /* Sprawdź stan przycisku po czasie debouncingu */
    int val = gpio_pin_get(gpio_dev, D14_PIN);
    if (val == 0) {
        /* Przycisk jest nadal wciśnięty, przełącz stan diody LED */
        gpio_pin_toggle(gpio_dev, D15_PIN);
    }
}

/* Funkcja wywoływana przy przerwaniu od przycisku */
static void button_pressed_isr(const struct device *dev, struct gpio_callback *cb,
                               uint32_t pins)
{
    /* Uruchom pracę opóźnioną do obsługi debouncingu */
    k_work_schedule(&button_work, K_MSEC(DEBOUNCE_DELAY_MS));
}

int main()
{
    int ret;

    /* Pobierz urządzenie GPIO */
    gpio_dev = DEVICE_DT_GET(GPIO_NODE);
    if (!device_is_ready(gpio_dev)) {
        printk("GPIO device not ready\n");
        return 1;
    }

    /* Skonfiguruj pin D15 jako wyjście */
    ret = gpio_pin_configure(gpio_dev, D15_PIN, D15_FLAGS);
    if (ret < 0) {
        printk("Failed to configure D15 pin\n");
        return 1;
    }

    /* Skonfiguruj pin D14 jako wejście z rezystorem podciągającym i przerwaniem */
    ret = gpio_pin_configure(gpio_dev, D14_PIN, D14_FLAGS);
    if (ret < 0) {
        printk("Failed to configure D14 pin\n");
        return 1;
    }

    /* Ustaw przerwanie na opadające zbocze (wciśnięcie przycisku) */
    ret = gpio_pin_interrupt_configure(gpio_dev, D14_PIN, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0) {
        printk("Failed to configure interrupt on D14 pin\n");
        return 1;
    }

    /* Inicjalizuj pracę opóźnioną do debouncingu */
    k_work_init_delayable(&button_work, button_pressed_work_handler);

    /* Dodaj callback do obsługi przerwania */
    gpio_init_callback(&button_cb_data, button_pressed_isr, BIT(D14_PIN));
    gpio_add_callback(gpio_dev, &button_cb_data);

    printk("System initialized\n");

    while (1) {
        k_sleep(K_FOREVER);
    }
    return 0;
}
