#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#define USER_LED_GPIO_NODE DT_NODELABEL(gpio1)  // GPIO dla LED
#define BUTTON_GPIO_NODE DT_NODELABEL(gpio5)   // GPIO dla przycisku
#define LED_PIN 9                              // Pin LED
#define BUTTON_PIN 0                           // Pin przycisku
#define DEBOUNCE_DELAY_MS 50                   // Czas debounce w milisekundach

// Globalne wskaźniki do urządzeń GPIO
static const struct device *led;
static const struct device *button;

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

    static bool led_state = false; // Lokalny stan LED

    // Zmiana stanu LED
    led_state = !led_state;
    gpio_pin_set(led, LED_PIN, led_state);

    // Informacja diagnostyczna
    printk("Przycisk wciśnięty! Zmieniam stan LED na: %d\n", led_state);
}

// Konfiguracja LED
void configure_led(void)
{
    led = DEVICE_DT_GET(USER_LED_GPIO_NODE);
    if (!device_is_ready(led)) {
        printk("Błąd: GPIO dla LED nie jest gotowe!\n");
        return;
    }

    // Konfiguracja pinu LED jako wyjście
    int ret = gpio_pin_configure(led, LED_PIN, GPIO_OUTPUT | GPIO_ACTIVE_LOW);
    if (ret < 0) {
        printk("Błąd: nie udało się skonfigurować pinu LED!\n");
    }

    gpio_pin_set(led, LED_PIN, 0); // Wyłącz LED na start
}

// Konfiguracja przycisku
void configure_button(void)
{
    button = DEVICE_DT_GET(BUTTON_GPIO_NODE);
    if (!device_is_ready(button)) {
        printk("Błąd: GPIO dla przycisku nie jest gotowe!\n");
        return;
    }

    // Konfiguracja pinu przycisku jako wejście z przerwaniami
    int ret = gpio_pin_configure(button, BUTTON_PIN, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Błąd: nie udało się skonfigurować pinu przycisku!\n");
        return;
    }

    // Konfiguracja przerwań dla przycisku na zbocze opadające
    ret = gpio_pin_interrupt_configure(button, BUTTON_PIN, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0) {
        printk("Błąd: nie udało się skonfigurować przerwania dla przycisku!\n");
        return;
    }

    // Zarejestruj callback do obsługi przerwań
    gpio_init_callback(&button_cb_data, button_pressed, BIT(BUTTON_PIN));
    gpio_add_callback(button, &button_cb_data);

    printk("Przycisk skonfigurowany na pinie %d\n", BUTTON_PIN);
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
