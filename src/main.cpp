#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

/* Definicje dla pinów */
#define GPIO_NODE DT_NODELABEL(gpio1)
#define TRIG_PIN 17
#define ECHO_PIN 16
#define BUZZER_PIN 21

#define TRIG_FLAGS (GPIO_OUTPUT)
#define ECHO_FLAGS (GPIO_INPUT)
#define BUZZER_FLAGS (GPIO_OUTPUT)

/* Parametry dla dźwięku buzzera */
#define MIN_DISTANCE_CM 10     // Minimalna odległość do aktywacji buzzera
#define MAX_DISTANCE_CM 100    // Maksymalna odległość, przy której buzzer działa z przerwami
#define BUZZER_PULSE_MS 100    // Czas impulsu buzzera
#define BUZZER_OFF_MS 300      // Czas wyłączenia buzzera

/* Inicjalizacja urządzeń */
const struct device *gpio_dev;

/* Funkcja do mierzenia odległości */
int measure_distance() {
    uint32_t start, end;
    
    // Wysyłanie impulsu przez TRIG
    gpio_pin_set(gpio_dev, TRIG_PIN, 1);
    k_busy_wait(10);  // 10 mikrosekund
    gpio_pin_set(gpio_dev, TRIG_PIN, 0);

    // Czekanie na odbiór sygnału na pinie ECHO
    while (gpio_pin_get(gpio_dev, ECHO_PIN) == 0);
    start = k_cycle_get_32();

    while (gpio_pin_get(gpio_dev, ECHO_PIN) == 1);
    end = k_cycle_get_32();

    // Obliczenie odległości w centymetrach
    uint32_t duration = end - start;
    int distance_cm = (duration * 34300) / (2 * sys_clock_hw_cycles_per_sec());

    return distance_cm;
}

/* Funkcja kontrolująca buzzer na podstawie odległości */
void control_buzzer(int distance) {
    if (distance <= MIN_DISTANCE_CM) {
        // Przy bardzo małej odległości buzzer działa ciągle
        gpio_pin_set(gpio_dev, BUZZER_PIN, 1);
    } else if (distance <= MAX_DISTANCE_CM) {
        // Dla większej odległości buzzer działa z przerwami
        gpio_pin_set(gpio_dev, BUZZER_PIN, 1);
        k_msleep(BUZZER_PULSE_MS);
        gpio_pin_set(gpio_dev, BUZZER_PIN, 0);
        k_msleep(BUZZER_OFF_MS);
    } else {
        // Buzzer wyłączony przy większych odległościach
        gpio_pin_set(gpio_dev, BUZZER_PIN, 0);
    }
}

int main() {
    int ret;

    /* Inicjalizacja GPIO */
    gpio_dev = DEVICE_DT_GET(GPIO_NODE);
    if (!device_is_ready(gpio_dev)) {
        printk("GPIO device not ready\n");
        return 1;
    }

    /* Konfiguracja pinów */
    ret = gpio_pin_configure(gpio_dev, TRIG_PIN, TRIG_FLAGS);
    if (ret < 0) {
        printk("Failed to configure TRIG pin\n");
        return 1;
    }

    ret = gpio_pin_configure(gpio_dev, ECHO_PIN, ECHO_FLAGS);
    if (ret < 0) {
        printk("Failed to configure ECHO pin\n");
        return 1;
    }

    ret = gpio_pin_configure(gpio_dev, BUZZER_PIN, BUZZER_FLAGS);
    if (ret < 0) {
        printk("Failed to configure BUZZER pin\n");
        return 1;
    }

    printk("System initialized\n");

    while (1) {
        int distance = measure_distance();
        printk("Odległość: %d cm\n", distance);

        control_buzzer(distance);
        k_msleep(500);  // Pomiary co pół sekundy
    }
    return 0;
}
