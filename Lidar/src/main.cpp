#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "engine.h"
#include "nrf24.h"
#include "hcsr04.h"
#include "lidar.h"

LOG_MODULE_REGISTER(robot, LOG_LEVEL_INF);


// Stack sizes and priorities
#define STACK_SIZE 1024
#define JOYSTICK_THREAD_PRIORITY 5
#define MOTOR_THREAD_PRIORITY 5
#define LIDAR_THREAD_PRIORITY 5

// Global objects
const struct device* spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi1));
const struct device* gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));

NRF24 radio(gpio_dev, spi_dev);
Engine engine(gpio_dev);
Lidar lidar(gpio_dev, 26, 27);

DataPacket current_joystick_data = {0, 0, 0};

// Thread declarations
void joystick_thread(void *, void *, void *);
void motor_thread(void *, void *, void *);
void lidar_thread(void *, void *, void *);

// Thread stack declarations
K_THREAD_STACK_DEFINE(joystick_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(lidar_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(motor_stack, STACK_SIZE);

struct k_thread joystick_thread_data;
struct k_thread motor_thread_data;
struct k_thread lidar_thread_data;

// Mutex for shared resources
struct k_mutex data_mutex;
struct k_mutex radio_mutex;
struct k_mutex lidar_mutex;


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

    if (lidar.init() != 0) {
        LOG_ERR("Failed to initialize lidar");
        return -1;
    }

    radio.test_registers();

    k_mutex_init(&data_mutex);
    k_mutex_init(&radio_mutex);
    k_mutex_init(&lidar_mutex);

    k_thread_create(&joystick_thread_data, joystick_stack, STACK_SIZE,
                    joystick_thread, NULL, NULL, NULL,
                    JOYSTICK_THREAD_PRIORITY, 0, K_NO_WAIT);


    k_thread_create(&motor_thread_data, motor_stack, STACK_SIZE,
                    motor_thread, NULL, NULL, NULL,
                    MOTOR_THREAD_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&lidar_thread_data, lidar_stack, STACK_SIZE,
                lidar_thread, NULL, NULL, NULL,
                LIDAR_THREAD_PRIORITY, 0, K_NO_WAIT);

    while (1) {
        k_sleep(K_FOREVER);
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
        DataPacket joystick = current_joystick_data;
        k_mutex_unlock(&data_mutex);

        engine.controlMotors(joystick);
        k_sleep(K_MSEC(50));
        
    }
}

// Lidar thread: Control servo and measure distance
void lidar_thread(void *a, void *b, void *c) {
    LidarData lidarData;

    while (1) {
        for (uint8_t angle = 0; angle <= 180; angle+=2) {
            lidar.move_to_angle(angle);
            uint16_t distance = lidar.measure_distance();

            lidarData.angle = angle;
            lidarData.distance = distance;

            k_mutex_lock(&radio_mutex, K_FOREVER);
            if (radio.send_ack_payload(reinterpret_cast<uint8_t*>(&lidarData), sizeof(lidarData)) != 0) {
                LOG_ERR("Failed to send ack payload");
            }
            k_mutex_unlock(&radio_mutex);

            k_sleep(K_MSEC(100));
        }

        for (uint8_t angle = 180; angle >= 2; angle-=2) {
            lidar.move_to_angle(angle);
            uint16_t distance = lidar.measure_distance();

            lidarData.angle = angle;
            lidarData.distance = distance;

            k_mutex_lock(&radio_mutex, K_FOREVER);
            if (radio.send_ack_payload(reinterpret_cast<uint8_t*>(&lidarData), sizeof(lidarData)) != 0) {
                LOG_ERR("Failed to send ack payload");
            }
            k_mutex_unlock(&radio_mutex);

            k_sleep(K_MSEC(100));
        }
    }
}

