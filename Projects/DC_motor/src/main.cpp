#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>

/* Pobierz konfiguracje PWM na podstawie aliasów z DTS */
static const struct pwm_dt_spec pwm_d4_spec = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
static const struct device *pwm_d3_dev = DEVICE_DT_GET(DT_ALIAS(pwm_d3));

int main(void)
{
    if (!device_is_ready(pwm_d4_spec.dev)) {
        printk("PWM D4 device not ready\n");
        return -1;
    }

    if (!device_is_ready(pwm_d3_dev)) {
        printk("PWM D3 device not ready\n");
        return -1;
    }

    uint32_t period_ns = 1000000; // 1 ms => 1 kHz
    uint32_t max_duty_ns = period_ns; // Maksymalna wartość wypełnienia
    uint32_t duty_ns = 0; // Początkowe wypełnienie
    bool increasing = true; // Zmienna kontrolująca kierunek zmiany wypełnienia

    uint32_t period = PWM_MSEC(20);  // Okres w nanosekundach (PWM D3)
    uint32_t max_pulse = period;     // Maksymalne wypełnienie (PWM D3)
    uint32_t pulse = 0; // Początkowe wypełnienie (PWM D3)
    
    printk("Starting PWM fade on D3 and D4...\n");

    while (1) {
        // Ustawienie PWM dla D4
        int ret = pwm_set_dt(&pwm_d4_spec, period_ns, duty_ns);
        if (ret) {
            printk("Failed to set PWM on D4: %d\n", ret);
            return -1;
        }

        // Ustawienie PWM dla D3
        ret = pwm_set(pwm_d3_dev, 0, period, pulse, PWM_POLARITY_NORMAL);
        if (ret) {
            printk("Failed to set PWM on D3: %d\n", ret);
            return -1;
        }

        // Zmieniaj wypełnienie cyklicznie (rozjaśnianie/gaszenie)
        if (increasing) {
            duty_ns += period_ns / 100; // Zwiększ o 1% okresu
            pulse += period / 100;     // Zwiększ o 1% okresu dla D3
            if (duty_ns >= max_duty_ns || pulse >= max_pulse) {
                increasing = false; // Zmień kierunek na zmniejszanie
            }
        } else {
            if (duty_ns > period_ns / 100) {
                duty_ns -= period_ns / 100; // Zmniejsz o 1% okresu
            } else {
                duty_ns = 0;
            }
            if (pulse > period / 100) {
                pulse -= period / 100; // Zmniejsz o 1% okresu dla D3
            } else {
                pulse = 0;
            }
            if (duty_ns == 0 && pulse == 0) {
                increasing = true; // Zmień kierunek na zwiększanie
            }
        }

        // Krótka pauza dla płynnego efektu
        k_sleep(K_MSEC(50));
    }

    return 0;
}
