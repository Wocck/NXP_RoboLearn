#ifndef LIDAR_H
#define LIDAR_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include "hcsr04.h"

#define SERWO_PWM_PIN 10
#define SERVO_ALIAS DT_ALIAS(pwm_d5)

class Lidar {
public:
    Lidar(const struct device* gpio_dev, uint8_t trig_pin, uint8_t echo_pin);

    int init();

    int move_to_angle(uint8_t angle);

    uint16_t measure_distance();

private:
    HCSR04 sensor;
    const struct pwm_dt_spec servo;              

    static constexpr uint32_t SERVO_PERIOD_NSEC = 20000 * 1000;
    static constexpr uint32_t SERVO_MIN_PULSE_NSEC = 500 * 1000;
    static constexpr uint32_t SERVO_MAX_PULSE_NSEC = 2500 * 1000;
};

#endif // LIDAR_H
