#include "line_follower.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(line_follower, LOG_LEVEL_INF);

LineFollower::LineFollower(const struct device* gpio_dev, uint8_t sensor_pin, Engine& engine)
    : gpio_dev(gpio_dev), sensor_pin(sensor_pin), engine(engine),
      kp(1.0), ki(0.0), kd(0.5), previous_error(0), integral(0) {}

int LineFollower::init() {
    if (!device_is_ready(gpio_dev)) {
        LOG_ERR("GPIO device not ready");
        return -1;
    }

    if(engine.init() != 0) {
        LOG_ERR("Failed to initialize engine");
        return -1;
    }

    if (gpio_pin_configure(gpio_dev, sensor_pin, GPIO_INPUT) < 0) {
        LOG_ERR("Failed to configure sensor pin %d", sensor_pin);
        return -1;
    }

    LOG_INF("Line follower initialized");
    return 0;
}

bool LineFollower::isLineDetected() {
    return gpio_pin_get(gpio_dev, sensor_pin) == 1;
}

float LineFollower::calculatePID(float error) {
    integral += error;
    float derivative = error - previous_error;
    previous_error = error;

    return kp * error + ki * integral + kd * derivative;
}

void LineFollower::followLine() {
    while (true) {
        bool lineDetected = isLineDetected();

        float error = lineDetected ? 0.0 : -1.0;
        float correction = calculatePID(error);

        DataPacket adjustment = {
            .joystickX = static_cast<int8_t>(correction * 50),
            .joystickY = 10
        };

        engine.controlMotors(adjustment);

        k_sleep(K_MSEC(50));
    }
}
