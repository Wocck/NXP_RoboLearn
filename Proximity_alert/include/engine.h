#ifndef ENGINE_H
#define ENGINE_H

#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "datapacket.h"


class Engine {
public:
    Engine(const struct device* gpio);

    bool init();
    void setMotorSpeed(uint32_t pulse_ns, const pwm_dt_spec &pwm_spec);
    void setMotorDirection(uint8_t in1, uint8_t in2, bool forward);
    void controlMotors(const DataPacket &joystickData);

private:
    const struct pwm_dt_spec motor_a; /**< Specyfikacja PWM dla silnika A (ENA). */
    const struct pwm_dt_spec motor_b; /**< Specyfikacja PWM dla silnika B (ENB). */

    const struct device *gpio_dev; /**< Wskaźnik do kontrolera GPIO. */
    uint32_t mapSpeedToPulse(uint8_t speed);
};

#endif // ENGINE_H
