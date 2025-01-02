#ifndef LINE_FOLLOWER_H
#define LINE_FOLLOWER_H

#include <zephyr/drivers/gpio.h>
#include "engine.h"

class LineFollower {
public:
    LineFollower(const struct device* gpio_dev, uint8_t sensor_pin, Engine& engine);

    int init();
    void followLine();
    bool isLineDetected();
private:
    const struct device* gpio_dev;
    uint8_t sensor_pin;
    Engine& engine;

    float kp;  // Proportional gain
    float ki;  // Integral gain
    float kd;  // Derivative gain

    float previous_error;  // Previous error value for derivative calculation
    float integral;        // Integral accumulator

   
    float calculatePID(float error);
};

#endif // LINE_FOLLOWER_H