#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

#define MIN_PERIOD PWM_SEC(1U) / 128U
#define MAX_PERIOD PWM_SEC(1U)

int main(void)
{
	uint32_t max_period;
	int ret;

	printk("PWM LED fade in/out\n");

	if (!pwm_is_ready_dt(&pwm_led0)) {
		printk("Error: PWM device %s is not ready\n", pwm_led0.dev->name);
		return 0;
	}

	/*
	 * Kalibracja maksymalnego okresu tak samo jak w oryginale.
	 * Po znalezieniu max_period użyjemy go jako stałego okresu.
	 */
	printk("Calibrating for channel %d...\n", pwm_led0.channel);
	max_period = MAX_PERIOD;
	while (pwm_set_dt(&pwm_led0, max_period, max_period / 2U)) {
		max_period /= 2U;
		if (max_period < (4U * MIN_PERIOD)) {
			printk("Error: PWM device "
			       "does not support a period at least %lu\n",
			       4U * MIN_PERIOD);
			return 0;
		}
	}

	printk("Done calibrating; using fixed period %u nsec\n", max_period);

	/*
	 * Teraz ustalimy stały okres i będziemy zmieniać tylko duty cycle.
	 * Wartość duty cycle będzie zmieniana w pętli od 0 do max_period
	 * i z powrotem, dając efekt płynnego rozjaśniania i ściemniania diody.
	 */

	uint32_t duty = 0;
	bool increasing = true; // na początku zwiększamy jasność
	uint32_t step = max_period / 100; // krok zmiany duty cycle (1%)

	while (1) {
		// Ustawienie PWM z wybranym duty cycle
		ret = pwm_set_dt(&pwm_led0, max_period, duty);
		if (ret) {
			printk("Error %d: failed to set pulse width\n", ret);
			return 0;
		}

		// Krótka pauza, aby zmiana była płynna
		k_sleep(K_MSEC(50));

		// Aktualizacja duty cycle
		if (increasing) {
			duty += step;
			if (duty >= max_period) {
				duty = max_period;
				increasing = false; // osiągnęliśmy max, teraz zmniejszamy
			}
		} else {
			if (duty > step) {
				duty -= step;
			} else {
				duty = 0;
				increasing = true; // osiągnęliśmy 0, czas znów rozjaśniać
			}
		}
	}

	return 0;
}
