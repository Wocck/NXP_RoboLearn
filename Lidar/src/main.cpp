#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "engine.h"
#include "nrf24.h"
#include "hcsr04.h"

#define SERVO_ALIAS DT_ALIAS(pwm_d3)

const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(SERVO_ALIAS);

#define SERVO_PERIOD_NSEC (20000 * 1000)  // 20ms in nanoseconds
#define SERVO_MIN_PULSE_NSEC (1000 * 1000)  // 1ms in nanoseconds
#define SERVO_MAX_PULSE_NSEC (2000 * 1000)  // 2ms in nanoseconds

LOG_MODULE_REGISTER(robot, LOG_LEVEL_INF);

// Robot constances
#define COLLISION_DST 20

// Stack sizes and priorities
#define STACK_SIZE 1024
#define JOYSTICK_THREAD_PRIORITY 5
#define MOTOR_THREAD_PRIORITY 5

// Global objects
const struct device* spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi1));
const struct device* gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));

NRF24 radio(gpio_dev, spi_dev);
Engine engine(gpio_dev);
HCSR04 sensor(gpio_dev, 26, 27);

DataPacket current_joystick_data = {0, 0, 0};
bool obstacle_detected = false;

// Thread declarations
void joystick_thread(void *, void *, void *);
void motor_thread(void *, void *, void *);

// Thread stack declarations
K_THREAD_STACK_DEFINE(joystick_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(proximity_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(motor_stack, STACK_SIZE);

struct k_thread joystick_thread_data;
struct k_thread motor_thread_data;

// Mutex for shared resources
struct k_mutex data_mutex;
struct k_mutex radio_mutex;


int main(void) {
    LOG_INF("Starting robot system");

    if (!device_is_ready(spi_dev) || !device_is_ready(gpio_dev)) {
        LOG_ERR("SPI or GPIO device not ready");
        return -1;
    }

    radio.reset(0);
    if (radio.init() != 0) {
        LOG_ERR("Failed to initialize components");
        return -1;
    }

    if (radio.configure_irq() != 0) {
        LOG_ERR("Failed to configure IRQ");
        return -1;
    }

    if (engine.init() != 0) {
        LOG_ERR("Failed to initialize engine");
        return -1;
    }

    if (sensor.init() != 0) {
        LOG_ERR("Failed to initialize sensor");
        return -1;
    }

    radio.test_registers();

    k_mutex_init(&data_mutex);
    k_mutex_init(&radio_mutex);

    // Create threads
    k_thread_create(&joystick_thread_data, joystick_stack, STACK_SIZE,
                    joystick_thread, NULL, NULL, NULL,
                    JOYSTICK_THREAD_PRIORITY, 0, K_NO_WAIT);


    k_thread_create(&motor_thread_data, motor_stack, STACK_SIZE,
                    motor_thread, NULL, NULL, NULL,
                    MOTOR_THREAD_PRIORITY, 0, K_NO_WAIT);

    while (1) {
        k_sleep(K_SECONDS(1));
    }

    return 0;
}

// Joystick thread: Fetch joystick data from NRF24
void joystick_thread(void *a, void *b, void *c) {
    while (1) {
        if (radio.is_receiving()) {
            k_mutex_lock(&radio_mutex, K_FOREVER);
            DataPacket packet = radio.get_current_packet();
            k_mutex_unlock(&radio_mutex);

            printk("Joystick data: X=%d, Y=%d, Button=%d\n", packet.joystickX, packet.joystickY, packet.buttonPressed);

            k_mutex_lock(&data_mutex, K_FOREVER);
            current_joystick_data = packet;
            k_mutex_unlock(&data_mutex);
        }
        k_sleep(K_MSEC(50));
    }
}

// Motor thread: Control motors based on joystick and proximity data
void motor_thread(void *a, void *b, void *c) {
    while (1) {
        k_mutex_lock(&data_mutex, K_FOREVER);
        bool obstacle = obstacle_detected;
        DataPacket joystick = current_joystick_data;
        k_mutex_unlock(&data_mutex);

        if (obstacle) {
            if (engine.is_move_forward(joystick)) {
                engine.evasive_maneuver();
                engine.stop();
            } else {
                engine.controlMotors(joystick);
            }
        } else {
            engine.controlMotors(joystick);
        }

        k_sleep(K_MSEC(50));
    }
}
