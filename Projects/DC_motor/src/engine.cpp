#include "engine.h"

/* Definicje pinów GPIO1 */
#define MOTOR_IN1_PIN 11
#define MOTOR_IN2_PIN 10
#define MOTOR_IN3_PIN 18
#define MOTOR_IN4_PIN 19

/* Definicje aliasów PWM z Device Tree */
#define PWM_A_ALIAS DT_ALIAS(pwm_d3)     // ENA
#define PWM_B_ALIAS DT_ALIAS(pwm_led0)   // ENB

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

    setMotorDirectionA(true);
    setMotorDirectionB(true);

    /* Ustawienie początkowego wypełnienia PWM na 0% (wyłączone silniki) */
    setMotorSpeedA(0);
    setMotorSpeedB(0);

    printk("Engine initialized successfully\n");
    return true;
}

void Engine::setMotorSpeedA(uint32_t pulse_ns) {
    int ret = pwm_set_dt(&pwm_a_spec, pulse_ns, 0);
    if (ret < 0) {
        printk("Failed to set PWM on A (ENA): %d\n", ret);
    }
}

void Engine::setMotorSpeedB(uint32_t pulse_ns) {
    int ret = pwm_set_dt(&pwm_b_spec, pulse_ns, 0);
    if (ret < 0) {
        printk("Failed to set PWM on B (ENB): %d\n", ret);
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

uint32_t Engine::mapJoystickToPulse(int8_t value, uint32_t period_ns) {
    // Klamrowanie wartości joysticka do zakresu -90 do 90
    if (value > 90) value = 90;
    if (value < -90) value = -90;

    if (value == 0) {
        return 0;
    } else {
        // Obliczanie wypełnienia PWM proporcjonalnie do wartości joysticka
        // Dodajemy minimalne wypełnienie dla większej rozdzielczości
        const uint32_t min_pulse_ns = period_ns / 180; // ~0.56% duty cycle

        return min_pulse_ns + (abs(value) * (period_ns - min_pulse_ns)) / 90;
    }
}

void Engine::controlFromJoystick(const DataPacket& data) {
    int joystickX = data.joystickX; // -90 do 90
    int joystickY = data.joystickY; // -90 do 90

    // Obliczanie wartości dla każdego silnika
    int left = joystickY + joystickX;   // Silnik B (lewy)
    int right = joystickY - joystickX;  // Silnik A (prawy)

    // Klamrowanie wartości do zakresu -90 do 90
    if (left > 90) left = 90;
    if (left < -90) left = -90;
    if (right > 90) right = 90;
    if (right < -90) right = -90;

    // Obliczanie wypełnienia PWM
    uint32_t left_pulse = mapJoystickToPulse(left, pwm_b_spec.period);
    uint32_t right_pulse = mapJoystickToPulse(right, pwm_a_spec.period);

    // Ustawianie kierunku i prędkości silnika B (lewy)
    if (left > 0) {
        setMotorDirectionB(true); // Forward
        setMotorSpeedB(left_pulse);
    } else if (left < 0) {
        setMotorDirectionB(false); // Backward
        setMotorSpeedB(left_pulse);
    } else {
        // Stop silnika B
        setMotorSpeedB(0);
    }

    // Ustawianie kierunku i prędkości silnika A (prawy)
    if (right > 0) {
        setMotorDirectionA(true); // Forward
        setMotorSpeedA(right_pulse);
    } else if (right < 0) {
        setMotorDirectionA(false); // Backward
        setMotorSpeedA(right_pulse);
    } else {
        // Stop silnika A
        setMotorSpeedA(0);
    }

    // Debugowanie: Logowanie prędkości i kierunków silników
    printk("Left Motor: %s at %u ns PWM\n",
           (left > 0) ? "Forward" : (left < 0) ? "Backward" : "Stopped",
           left != 0 ? left_pulse : 0);

    printk("Right Motor: %s at %u ns PWM\n",
           (right > 0) ? "Forward" : (right < 0) ? "Backward" : "Stopped",
           right != 0 ? right_pulse : 0);
}