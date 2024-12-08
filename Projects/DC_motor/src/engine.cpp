#include "engine.h"

/* Definicje pinów GPIO1 */
#define MOTOR_IN1_PIN 11
#define MOTOR_IN2_PIN 10
#define MOTOR_IN3_PIN 18
#define MOTOR_IN4_PIN 19

/* Definicje aliasów PWM z Device Tree */
#define PWM_A_ALIAS DT_ALIAS(pwm_d3)     // ENA
#define PWM_B_ALIAS DT_ALIAS(pwm_led0)   // ENB

#define PERIOD_NS 50000

Engine::Engine()
    : pwm_a_spec(PWM_DT_SPEC_GET(PWM_A_ALIAS)),
      pwm_b_spec(PWM_DT_SPEC_GET(PWM_B_ALIAS)),
      gpio_dev(DEVICE_DT_GET(DT_NODELABEL(gpio1)))
{}

bool Engine::init() {
    /* Sprawdzenie, czy urządzenia PWM są gotowe */
    if (!device_is_ready(pwm_a_spec.dev)) {
        printk("PWM A device not ready\n");
        return false;
    }

    if (!device_is_ready(pwm_b_spec.dev)) {
        printk("PWM B device not ready\n");
        return false;
    }

    /* Sprawdzenie, czy kontroler GPIO jest gotowy */
    if (!device_is_ready(gpio_dev)) {
        printk("GPIO device not ready\n");
        return false;
    }

    /* Konfiguracja pinów GPIO jako wyjścia, początkowo wyłączone */
    int ret;

    ret = gpio_pin_configure(gpio_dev, MOTOR_IN1_PIN, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Error configuring GPIO_IN1 (D2): %d\n", ret);
        return false;
    }

    ret = gpio_pin_configure(gpio_dev, MOTOR_IN2_PIN, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Error configuring GPIO_IN2 (D5): %d\n", ret);
        return false;
    }

    ret = gpio_pin_configure(gpio_dev, MOTOR_IN3_PIN, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Error configuring GPIO_IN3 (D6): %d\n", ret);
        return false;
    }

    ret = gpio_pin_configure(gpio_dev, MOTOR_IN4_PIN, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Error configuring GPIO_IN4 (D7): %d\n", ret);
        return false;
    }
    setMotorSpeedA(0);
    setMotorSpeedB(0);

    printk("Engine initialized successfully\n");
    return true;
}

void Engine::setMotorSpeedA(uint32_t pulse_ns) {
    if (pulse_ns > PERIOD_NS) {
        pulse_ns = PERIOD_NS; // Ogranicz wartość do maksymalnego okresu
    }

    int ret = pwm_set_dt(&pwm_a_spec, PERIOD_NS, pulse_ns);
    if (ret < 0) {
        printk("Failed to set PWM A: %d\n", ret);
    }
}

void Engine::setMotorSpeedB(uint32_t pulse_ns) {
    if (pulse_ns > PERIOD_NS) {
        pulse_ns = PERIOD_NS; // Ogranicz wartość do maksymalnego okresu
    }

    int ret = pwm_set_dt(&pwm_b_spec, PERIOD_NS, pulse_ns);
    if (ret < 0) {
        printk("Failed to set PWM B: %d\n", ret);
    }
}

void Engine::setMotorDirectionA(bool forward) {
    if (forward) {
        gpio_pin_set(gpio_dev, MOTOR_IN1_PIN, 1); // IN1 = HIGH
        gpio_pin_set(gpio_dev, MOTOR_IN2_PIN, 0); // IN2 = LOW
    } else {
        gpio_pin_set(gpio_dev, MOTOR_IN1_PIN, 0); // IN1 = LOW
        gpio_pin_set(gpio_dev, MOTOR_IN2_PIN, 1); // IN2 = HIGH
    }
}

void Engine::setMotorDirectionB(bool forward) {
    if (forward) {
        gpio_pin_set(gpio_dev, MOTOR_IN3_PIN, 1); // IN3 = HIGH
        gpio_pin_set(gpio_dev, MOTOR_IN4_PIN, 0); // IN4 = LOW
    } else {
        gpio_pin_set(gpio_dev, MOTOR_IN3_PIN, 0); // IN3 = LOW
        gpio_pin_set(gpio_dev, MOTOR_IN4_PIN, 1); // IN4 = HIGH
    }
}

uint32_t Engine::mapSpeedToPulse(uint8_t speed) {
    // Zakres wejściowy: 0 - 90
    if(speed == 0) {
        return 0;
    }
    const uint32_t min_pulse_ns = 35000; // Minimalne pulse_ns dla uruchomienia silnika
    const uint32_t max_pulse_ns = PERIOD_NS; // Maksymalne pulse_ns
    const uint8_t max_speed = 90;        // Maksymalna wartość wejściowa

    // Zabezpieczenie przed przekroczeniem zakresu
    if (speed > max_speed) {
        speed = max_speed;
    }

    // Mapowanie liniowe: pulse_ns = min_pulse_ns + (speed / max_speed) * (max_pulse_ns - min_pulse_ns)
    uint32_t pulse_ns = min_pulse_ns + ((max_pulse_ns - min_pulse_ns) * speed) / max_speed;

    return pulse_ns;
}

void Engine::controlMotors(const DataPacket &joystickData) {
    // Wartości z joysticka
    int8_t x = joystickData.joystickX;
    int8_t y = joystickData.joystickY;

    // Obliczanie wypełnienia PWM dla każdego silnika
    uint32_t pulseA = 0, pulseB = 0; // Wartości PWM
    bool directionA = true, directionB = true; // Kierunki (true = do przodu)

    // 1) Obracanie w miejscu
    if (y == 0 && x != 0) {
        // Dla obracania w miejscu jeden silnik do przodu, drugi do tyłu
        pulseA = mapSpeedToPulse(abs(x));
        pulseB = mapSpeedToPulse(abs(x));

        directionA = (x < 0); // Jeśli X < 0, prawy silnik do przodu
        directionB = (x > 0); // Jeśli X > 0, lewy silnik do przodu
    }
    // 2) Jazda w linii prostej
    else if (x == 0 && y != 0) {
        // Oba silniki z tą samą prędkością i kierunkiem
        pulseA = mapSpeedToPulse(abs(y));
        pulseB = mapSpeedToPulse(abs(y));

        directionA = (y > 0); // Jeśli Y > 0, jedziemy do przodu
        directionB = (y > 0);
    }
    // 3) Skręcanie podczas ruchu
    else if (x != 0 && y != 0) {
        // Dla skrętu różnicujemy prędkości obu silników
        int8_t speedA = y + x; // Dodajemy wpływ X na prawy silnik
        int8_t speedB = y - x; // Odejmujemy wpływ X na lewy silnik

        // Normalizacja prędkości do zakresu -90 do 90
        speedA = (speedA > 90) ? 90 : (speedA < -90 ? -90 : speedA);
        speedB = (speedB > 90) ? 90 : (speedB < -90 ? -90 : speedB);

        pulseA = mapSpeedToPulse(abs(speedA));
        pulseB = mapSpeedToPulse(abs(speedB));

        directionA = (speedA > 0); // Jeśli > 0, jedziemy do przodu
        directionB = (speedB > 0);
    }
    // 4) Stop (X == 0 i Y == 0)
    else {
        pulseA = 0;
        pulseB = 0;
        directionA = true; // Domyślnie kierunek do przodu
        directionB = true;
    }

    // Ustawienie prędkości i kierunków
    setMotorDirectionA(directionA);
    setMotorDirectionB(directionB);

    setMotorSpeedA(pulseA);
    setMotorSpeedB(pulseB);
}