#include "engine.h"

/* GPIO and PWM Definitions */
#define MOTOR_IN1_PIN 11
#define MOTOR_IN2_PIN 10
#define MOTOR_IN3_PIN 18
#define MOTOR_IN4_PIN 19

#define PWM_A_ALIAS DT_ALIAS(pwm_d3)     // ENA
#define PWM_B_ALIAS DT_ALIAS(pwm_led0)   // ENB

#define PERIOD_NS 50000
#define MIN_PULSE_NS 35000
#define MAX_PULSE_NS 45000
#define MAX_SPEED 90

Engine::Engine(const struct device* gpio)
    : motor_a(PWM_DT_SPEC_GET(PWM_A_ALIAS)),
      motor_b(PWM_DT_SPEC_GET(PWM_B_ALIAS)),
      gpio_dev(gpio) {}

bool Engine::init() {
    if (!device_is_ready(motor_a.dev) || !device_is_ready(motor_b.dev) || !device_is_ready(gpio_dev)) {
        printk("Device not ready\n");
        return false;
    }

    int pins[] = {MOTOR_IN1_PIN, MOTOR_IN2_PIN, MOTOR_IN3_PIN, MOTOR_IN4_PIN};
    for (int pin : pins) {
        if (gpio_pin_configure(gpio_dev, pin, GPIO_OUTPUT_INACTIVE) < 0) {
            printk("Error configuring GPIO pin %d\n", pin);
            return false;
        }
    }

    setMotorSpeed(0, motor_a);
    setMotorSpeed(0, motor_b);
    printk("Engine initialized successfully\n");
    return true;
}

void Engine::setMotorSpeed(uint32_t pulse_ns, const pwm_dt_spec &pwm_spec) {
    if (pwm_set_dt(&pwm_spec, PERIOD_NS, MIN(pulse_ns, PERIOD_NS)) < 0) {
        printk("Failed to set PWM\n");
    }
}

void Engine::setMotorDirection(uint8_t in1, uint8_t in2, bool forward) {
    gpio_pin_set(gpio_dev, in1, forward);
    gpio_pin_set(gpio_dev, in2, !forward);
}

uint32_t Engine::mapSpeedToPulse(uint8_t speed) {
    return (speed == 0) ? 0 : MIN_PULSE_NS + ((MAX_PULSE_NS - MIN_PULSE_NS) * MIN(speed, MAX_SPEED)) / MAX_SPEED;
}

void Engine::controlMotors(const DataPacket &joystickData) {
    int8_t x = joystickData.joystickX;
    int8_t y = joystickData.joystickY;

    uint32_t pulseA = 0, pulseB = 0;
    bool directionA = (y > 0);
    bool directionB = (y > 0);

    if (x == 0 && y == 0) {
        pulseA = 0;
        pulseB = 0;
    } 
    else if (y == 0 && x != 0) {
        pulseA = mapSpeedToPulse(abs(x)/5); // Slower turn
        pulseB = mapSpeedToPulse(abs(x)/5); // Slower turn

        directionA = (x < 0);
        directionB = (x > 0);
    }
    else if (x == 0 && y != 0) {
        pulseA = mapSpeedToPulse(abs(y));
        pulseB = mapSpeedToPulse(abs(y));

        directionA = (y > 0);
        directionB = (y > 0);
    }
    else {
        int baseSpeed = abs(abs(y) > abs(x) ? abs(y) : abs(x));
        int diff = abs(abs(y) - abs(x));
        if(x > 0) {
            if(diff == 0){
                pulseA = mapSpeedToPulse(baseSpeed/3);
                pulseB = mapSpeedToPulse(baseSpeed);
            } else {
                pulseA = mapSpeedToPulse(baseSpeed - diff);
                pulseB = mapSpeedToPulse(baseSpeed);
            }
        } else {
            if(diff == 0){
                pulseA = mapSpeedToPulse(baseSpeed);
                pulseB = mapSpeedToPulse(baseSpeed/3);
            } else {
                pulseA = mapSpeedToPulse(baseSpeed);
                pulseB = mapSpeedToPulse(baseSpeed - diff);
            }
        }
    }

    setMotorDirection(MOTOR_IN1_PIN, MOTOR_IN2_PIN, directionA);
    setMotorDirection(MOTOR_IN3_PIN, MOTOR_IN4_PIN, directionB);

    setMotorSpeed(pulseA, motor_a);
    setMotorSpeed(pulseB, motor_b);
}