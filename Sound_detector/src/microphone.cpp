#include "microphone.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(microphone, LOG_LEVEL_INF);

Microphone::Microphone(const struct device* adc_dev, uint8_t analog_pin)
    : adc_dev(adc_dev), analog_pin(analog_pin) {}

int Microphone::init() {
    if (!device_is_ready(adc_dev)) {
        LOG_ERR("ADC device not ready");
        return -1;
    }

    struct adc_channel_cfg channel_cfg = {
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = analog_pin,
        .differential     = 0,
    };
    int ret = adc_channel_setup(adc_dev, &channel_cfg);
    if (ret < 0) {
        LOG_ERR("ADC channel setup failed");
        return ret;
    }

    LOG_INF("Microphone initialized successfully (ADC channel %d)", analog_pin);
    return 0;
}

uint16_t Microphone::read_analog() {
    static __aligned(4) int16_t buffer;
    
    struct adc_sequence sequence = {
        .channels = BIT(analog_pin),
        .buffer = &buffer,
        .buffer_size = sizeof(buffer),
        .resolution = 12,
        .oversampling = 0,
    };

    uint32_t sum_of_squares = 0;
    const int samples = 100;
    for (int i = 0; i < samples; i++) {
        int ret = adc_read(adc_dev, &sequence);
        if (ret < 0) {
            LOG_ERR("Failed to read ADC, err=%d", ret);
            return 0;
        }
        sum_of_squares += static_cast<int32_t>(buffer) * buffer;
        k_sleep(K_USEC(100));
    }
    uint32_t rms = sqrt(sum_of_squares / samples);
    return static_cast<uint16_t>(rms);
}

float Microphone::calculate_rms_with_offset() {
    static __aligned(4) int16_t buffer;
    uint32_t sum_of_squares = 0;
    const int samples = 100;
    struct adc_sequence sequence = {
        .channels = BIT(analog_pin),
        .buffer = &buffer,
        .buffer_size = sizeof(buffer),
        .resolution = 12,
        .oversampling = 0,
    };

    for (int i = 0; i < samples; i++) {
        int ret = adc_read(adc_dev, &sequence);
        if (ret < 0) {
            LOG_ERR("Failed to read ADC, err=%d", ret);
            return 0;
        }

        int16_t adjusted_value = buffer - offset;
        sum_of_squares += adjusted_value * adjusted_value;

        k_sleep(K_USEC(100));
    }
    return sqrt(sum_of_squares / samples);
}

void Microphone::calibrate() {
    int sum = 0;
    for(int i = 0; i < 100; i++){
        sum += read_analog();
    }
    LOG_INF("Setting offset to: %d", sum/100);
    offset = sum/100;
}
