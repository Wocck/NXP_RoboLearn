#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "engine.h"
#include "line_follower.h"

LOG_MODULE_REGISTER(robot, LOG_LEVEL_INF);
const struct device* gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));

Engine engine(gpio_dev);
LineFollower lineFollower(gpio_dev, 16, engine);

int main(void) {
    LOG_INF("Starting robot system");
    
    if (lineFollower.init() != 0) {
        LOG_ERR("Failed to initialize line follower");
        return -1;
    }

    lineFollower.followLine();
    return 0;
}
