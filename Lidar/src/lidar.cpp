#include "lidar.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lidar, LOG_LEVEL_INF);

Lidar::Lidar(const struct device* gpio_dev, uint8_t trig_pin, uint8_t echo_pin) :
    sensor(gpio_dev, trig_pin, echo_pin),
    servo(PWM_DT_SPEC_GET(SERVO_ALIAS)) {}

int Lidar::init() {
    if (!device_is_ready(servo.dev)) {
        LOG_ERR("PWM device not ready");
        return -1;
    }

    if (sensor.init() != 0) {
        LOG_ERR("Failed to initialize HC-SR04 sensor");
        return -1;
    }

    LOG_INF("Lidar initialized successfully");
    return 0;
}

int Lidar::move_to_angle(uint8_t angle) {
    if (angle > 180) {
        LOG_ERR("Invalid angle: %d", angle);
        return -1;
    }

    // Oblicz szerokość impulsu dla danego kąta
    uint32_t pulse_width = SERVO_MIN_PULSE_NSEC +
                           ((SERVO_MAX_PULSE_NSEC - SERVO_MIN_PULSE_NSEC) * angle) / 180;

    int ret = pwm_set_dt(&servo, SERVO_PERIOD_NSEC, pulse_width);
    if (ret != 0) {
        LOG_ERR("Failed to set servo to angle %d", angle);
        return ret;
    }
    return 0;
}

uint16_t Lidar::measure_distance() {
    uint16_t distance = sensor.measureDistance();
    if (distance < 0) {
        LOG_ERR("Failed to measure distance");
        return distance;
    }
    return distance;
}