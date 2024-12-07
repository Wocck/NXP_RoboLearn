#include <zephyr/kernel.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

// Konfiguracja LED
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

// Funkcja callback wywoływana po osiągnięciu wartości top przez PIT
static void pit_top_callback(const struct device *dev, void *user_data)
{
    printk("PIT top value reached!\n");
    gpio_pin_toggle_dt(&led);
}

int main(void)
{
    // Uchwyty do urządzeń
    const struct device *pit_ch0 = DEVICE_DT_GET(DT_NODELABEL(pit0_channel0));
    if (!device_is_ready(pit_ch0)) {
        printk("PIT channel0 not ready\n");
        return -1;
    }

    if (!device_is_ready(led.port)) {
        printk("LED device not ready\n");
        return -1;
    }

    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
        printk("Failed to configure LED pin\n");
        return -1;
    }

    // Odczytanie częstotliwości wejściowej PIT
    uint32_t freq = counter_get_frequency(pit_ch0);
    printk("PIT frequency: %u Hz\n", freq);

    // Konfiguracja top i callbacka
    struct counter_top_cfg cfg = {
        .ticks = freq,        // maksymalna wartość (1s)
        .callback = pit_top_callback,
        .user_data = NULL,
        .flags = 0
    };

    // Ustaw top
    if (counter_set_top_value(pit_ch0, &cfg) < 0) {
        printk("Failed to set top value\n");
        return -1;
    }

    // Uruchom timer PIT
    if (counter_start(pit_ch0) < 0) {
        printk("Failed to start PIT\n");
        return -1;
    }

    while (1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
