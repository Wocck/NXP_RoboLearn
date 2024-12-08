#ifndef ENGINE_H
#define ENGINE_H

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

struct DataPacket {
    int8_t joystickX;      ///< Wartość osi X joysticka (-90 do 90, 0 oznacza stop)
    int8_t joystickY;      ///< Wartość osi Y joysticka (-90 do 90, 0 oznacza stop)
    uint8_t buttonPressed; ///< Status przycisku (0 - nie naciśnięty, 1 - naciśnięty)
};


class Engine {
public:
    Engine();

    bool init();
    void setMotorSpeedA(uint32_t pulse_ns);
    void setMotorSpeedB(uint32_t pulse_ns);
    void setMotorDirectionA(bool forward);
    void setMotorDirectionB(bool forward);
    void controlFromJoystick(const DataPacket& data);

private:
    const struct pwm_dt_spec pwm_a_spec; /**< Specyfikacja PWM dla silnika A (ENA). */
    const struct pwm_dt_spec pwm_b_spec; /**< Specyfikacja PWM dla silnika B (ENB). */

    const struct device *gpio_dev; /**< Wskaźnik do kontrolera GPIO. */
    uint32_t mapJoystickToPulse(int8_t value, uint32_t period_ns);
};

#endif // ENGINE_H
