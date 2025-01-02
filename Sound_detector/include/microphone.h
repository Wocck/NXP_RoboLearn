/**
 * @file microphone.h
 * @author Your Name
 * @date 2025-01-02
 * @brief Header file for the Microphone class, providing functionality to interface with the Iduino sound sensor.
 * This file contains the definition of the Microphone class and related methods to
 * initialize, configure, and read data from the Iduino sound sensor module. 
*/

#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <stdint.h>
#include <math.h>

#define DEFAULT_THRESHOLD 1000

class Microphone {
private:
    const struct device* adc_dev;    // ADC device for analog input
    uint8_t analog_pin;                 // ADC channel for A0
    uint16_t offset;

public:
    Microphone(const struct device* adc_dev, uint8_t analog_pin);
    int init();
    void set_threshold(uint16_t new_threshold);
    uint16_t read_analog();
    float calculate_rms_with_offset();
    void calibrate();
};

#endif // MICROPHONE_H
