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
#define MIN_DISTANCE_CM 10          // Minimalna odległość do aktywacji buzzera
#define MAX_DISTANCE_CM 20          // Maksymalna odległość, przy której buzzer działa z przerwami
#define SHORT_BUZZER_PULSE_MS 10    // Czas impulsu buzzera (blisko)
#define LONG_BUZZER_PULSE_MS 200    // Czas impulsu buzzera (daleko)
#define ZERO_DISTANCE 2000          // Dystans który odpowiada zerwoej odległości (należy dopasować do odczytów)

const struct device *gpio_dev;

int measure_distance() {
    uint32_t start, end;
    // gpio_pin_set(gpio_dev, TRIG_PIN, 0);
    // k_busy_wait(10);

    gpio_pin_set(gpio_dev, TRIG_PIN, 1);
    k_busy_wait(10);  // 10 mikrosekund
    gpio_pin_set(gpio_dev, TRIG_PIN, 0);

    // Czekanie na odbiór sygnału na pinie ECHO
    while (gpio_pin_get(gpio_dev, ECHO_PIN) == 0);
    start = k_cycle_get_32();

    while (gpio_pin_get(gpio_dev, ECHO_PIN) == 1);
    end = k_cycle_get_32();

    // Obliczenie odległości
    int duration = end - start;
    double time_s = (double)duration / sys_clock_hw_cycles_per_sec();
    printk("Czas: %.2f\n", time_s);
    int distance_cm = (int)((time_s * 34300) / 2);
    return distance_cm;
}


void control_buzzer(int distance) {
    if (distance >= ZERO_DISTANCE) {
        // Ciągłe włączenie buzzera, gdy obiekt jest bardzo blisko (>= 2000 cm)
        gpio_pin_set(gpio_dev, BUZZER_PIN, 1);
    } else if (distance <= MIN_DISTANCE_CM) {
        // Szybkie pikanie dla minimalnej odległości
        gpio_pin_set(gpio_dev, BUZZER_PIN, 1);
        k_msleep(SHORT_BUZZER_PULSE_MS);
        gpio_pin_set(gpio_dev, BUZZER_PIN, 0);
        k_msleep(SHORT_BUZZER_PULSE_MS);
    } else if (distance <= MAX_DISTANCE_CM) {
        // Wolniejsze pikanie dla większej odległości (w zakresie MAX_DISTANCE_CM)
        gpio_pin_set(gpio_dev, BUZZER_PIN, 1);
        k_msleep(LONG_BUZZER_PULSE_MS);
        gpio_pin_set(gpio_dev, BUZZER_PIN, 0);
        k_msleep(LONG_BUZZER_PULSE_MS);
    } else {
        // Wyłączenie buzzera, gdy obiekt jest poza zasięgiem określonych wartości
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
        if (distance != -1) {
            printk("Odległość: %d cm\n", distance);
            control_buzzer(distance);
        } else {
            printk("Błąd pomiaru\n");
        }
        k_msleep(500);
    }
    return 0;
}
