#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "engine.h"
#include "nrf24.h"
#include "microphone.h"

LOG_MODULE_REGISTER(robot, LOG_LEVEL_INF);


// Stack sizes and priorities
#define STACK_SIZE 2024
#define MICROPHONE_STACK_SIZE 4048
#define JOYSTICK_THREAD_PRIORITY 5
#define MOTOR_THREAD_PRIORITY 5
#define MICROPHONE_THREAD_PRIORITY 5

// Global objects
const struct device* spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi1));
const struct device* gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));

NRF24 radio(gpio_dev, spi_dev);
Engine engine(gpio_dev);
Microphone m1(DEVICE_DT_GET(DT_NODELABEL(adc1)), 9);
Microphone m2(DEVICE_DT_GET(DT_NODELABEL(adc1)), 10);
Microphone m3(DEVICE_DT_GET(DT_NODELABEL(adc1)), 6);

DataPacket current_joystick_data = {0, 0, 0};

// Thread declarations
void joystick_thread(void *, void *, void *);
void motor_thread(void *, void *, void *);
void microphone_thread(void *, void *, void *);

// Thread stack declarations
K_THREAD_STACK_DEFINE(joystick_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(microphone_stack, MICROPHONE_STACK_SIZE);
K_THREAD_STACK_DEFINE(motor_stack, STACK_SIZE);

struct k_thread joystick_thread_data;
struct k_thread motor_thread_data;
struct k_thread microphone_thread_data;

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

    if (m1.init() != 0) {
        LOG_ERR("Failed to initialize Microphone 1");
        return -1;
    }

    if (m2.init() != 0) {
        LOG_ERR("Failed to initialize Microphone 2");
        return -1;
    }

    if (m3.init() != 0) {
        LOG_ERR("Failed to initialize Microphone 3");
        return -1;
    }
    k_sleep(K_MSEC(2000));
    m1.calibrate();
    m2.calibrate();
    m3.calibrate();

    radio.test_registers();

    k_mutex_init(&data_mutex);
    k_mutex_init(&radio_mutex);

    k_thread_create(&joystick_thread_data, joystick_stack, STACK_SIZE,
                    joystick_thread, NULL, NULL, NULL,
                    JOYSTICK_THREAD_PRIORITY, 0, K_NO_WAIT);


    k_thread_create(&motor_thread_data, motor_stack, STACK_SIZE,
                    motor_thread, NULL, NULL, NULL,
                    MOTOR_THREAD_PRIORITY, 0, K_NO_WAIT);

    k_thread_create(&microphone_thread_data, microphone_stack, STACK_SIZE,
                    microphone_thread, NULL, NULL, NULL,
                    MICROPHONE_THREAD_PRIORITY, 0, K_NO_WAIT);

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

void microphone_thread(void *arg1, void *arg2, void *arg3)
{
    const double noise_threshold = 25.0; // Minimum total RMS to consider as sound
    const double max_angle = 180.0;    // Maximum angle (right microphone)

    while (true) {
        // Read RMS values from all three microphones
        double rms_left = m1.calculate_rms_with_offset();
        double rms_front = m2.calculate_rms_with_offset();
        double rms_right = m3.calculate_rms_with_offset();

        // Total RMS for noise detection
        double total_rms = rms_left + rms_front + rms_right;

        if (total_rms > noise_threshold) {
            // Normalize the RMS values
            double norm_left = rms_left / total_rms;
            double norm_front = rms_front / total_rms;
            double norm_right = rms_right / total_rms;

            // Calculate the angle dynamically
            double angle = (norm_left * 0 + norm_front * 90 + norm_right * max_angle);

            // Log the calculated direction and RMS values
            LOG_INF("Sound detected at %.2f° (L: %.2f, F: %.2f, R: %.2f)", angle, rms_left, rms_front, rms_right);
        } else {
            // No significant sound detected
            LOG_INF("No significant sound detected (L: %.2f, F: %.2f, R: %.2f)", rms_left, rms_front, rms_right);
        }

        // Small delay before the next measurement
        k_sleep(K_MSEC(50));
    }
}