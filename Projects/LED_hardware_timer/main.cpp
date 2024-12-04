#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

// Wskaźniki do urządzeń GPIO i PWM
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

// Deklaracja timera
static struct k_timer my_timer;

// Funkcja wywoływana po wygaśnięciu timera
void timer_expiry_function(struct k_timer *timer_id)
{
    gpio_pin_toggle_dt(&led);
    printk("Timer wyzwolony! Zmieniam stan LED\n");
}

// Funkcja inicjująca timer
void init_timer(void)
{
    k_timer_init(&my_timer, timer_expiry_function, NULL);
    k_timer_start(&my_timer, K_MSEC(1000), K_MSEC(1000)); // Wyzwalaj co 1000 ms
}

// Konfiguracja LED
void configure_led(void)
{
    if (!device_is_ready(led.port)) {
        printk("Błąd: GPIO dla LED nie jest gotowe!\n");
        return;
    }

    int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        printk("Błąd: nie udało się skonfigurować LED!\n");
    }
}

// Główna funkcja aplikacji
int main(void)
{
    printk("Start aplikacji z wykorzystaniem timera i PWM\n");

    // Konfiguracja LED
    configure_led();

    // Inicjalizacja timera
    init_timer();

    while (1) {
        k_sleep(K_FOREVER); // Proces główny pozostaje w uśpieniu
    }
    return 0;
}
