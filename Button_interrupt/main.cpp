#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#define DEBOUNCE_DELAY_MS 30 // Czas debounce w milisekundach

// Wskaźniki do urządzeń GPIO
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_NODELABEL(user_button), gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(green_led), gpios);

// Deklaracja struktury callback dla GPIO
static struct gpio_callback button_cb_data;

// Zmienna czasu ostatniego przerwania
static uint32_t last_press_time = 0;

// Callback obsługujący przerwanie przycisku
static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    uint32_t current_time = k_uptime_get_32(); // Pobierz czas w ms

    // Sprawdź, czy debounce jest w toku
    if ((current_time - last_press_time) < DEBOUNCE_DELAY_MS) {
        return; // Ignoruj przerwanie
    }

    last_press_time = current_time; // Zapisz czas przerwania

    gpio_pin_toggle_dt(&led);

    // Informacja diagnostyczna
    printk("Przycisk wciśnięty! Zmieniam stan LED\n");
}

// Konfiguracja LED
void configure_led(void)
{
    if (!device_is_ready(led.port)) {
        printk("Błąd: GPIO dla LED nie jest gotowe!\n");
        return;
    }

    // Konfiguracja LED jako wyjście
    int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
    if (ret < 0) {
        printk("Błąd: nie udało się skonfigurować LED!\n");
    }

    gpio_pin_set_dt(&led, 0); // Wyłącz LED na start
}

// Konfiguracja przycisku
void configure_button(void)
{
    if (!device_is_ready(button.port)) {
        printk("Błąd: GPIO dla przycisku nie jest gotowe!\n");
        return;
    }

    // Konfiguracja przycisku jako wejście z przerwaniami
    int ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret < 0) {
        printk("Błąd: nie udało się skonfigurować przycisku!\n");
        return;
    }

    // Konfiguracja przerwań dla przycisku na zbocze opadające
    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_INACTIVE);
    if (ret < 0) {
        printk("Błąd: nie udało się skonfigurować przerwania dla przycisku!\n");
        return;
    }

    // Zarejestruj callback do obsługi przerwań
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);

    printk("Przycisk skonfigurowany: %s\n", button.port->name);
}

// Główna funkcja aplikacji
int main(void)
{
    printk("Start aplikacji z debounce GPIO API w Zephyrze\n");

    // Konfiguracja LED i przycisku
    configure_led();
    configure_button();

    while (1) {
        k_sleep(K_FOREVER); // Proces główny pozostaje w uśpieniu
    }
    return 0;
}
