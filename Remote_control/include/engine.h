#ifndef ENGINE_H
#define ENGINE_H

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
    void setMotorSpeedA(uint32_t pulse_ns);
    void setMotorSpeedB(uint32_t pulse_ns);
    void setMotorDirectionA(bool forward);
    void setMotorDirectionB(bool forward);
    void controlMotors(const DataPacket &joystickData);

private:
    const struct pwm_dt_spec pwm_a_spec; /**< Specyfikacja PWM dla silnika A (ENA). */
    const struct pwm_dt_spec pwm_b_spec; /**< Specyfikacja PWM dla silnika B (ENB). */

    const struct device *gpio_dev; /**< Wskaźnik do kontrolera GPIO. */
    uint32_t mapSpeedToPulse(uint8_t speed);
};

#endif // ENGINE_H
