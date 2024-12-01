#ifndef NRF24_H
#define NRF24_H

#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>

// GPIOs
#define CE_GPIO_PIN 2 // GPIO_AD_B0_03 (D9)
#define CSN_GPIO_PIN 13 // GPIO_SD_B0_01 (D10)

struct DataPacket {
    int8_t joystickX;
    int8_t joystickY;
    uint8_t buttonPressed;
} __attribute__((packed));

class NRF24 {
private:
    const struct device* gpio_dev_1;
    const struct device* spi_dev;
    struct spi_config spi_cfg;
    uint8_t tx_buf[33];
    uint8_t rx_buf[33];

     int set_device(const struct device* spi);
    int write_register(uint8_t reg, const uint8_t* data, size_t len);
    int read_register(uint8_t reg, uint8_t* data, size_t len);

public:
    NRF24(const struct device* spi);
    void reset(uint8_t reg);
    int init();
    int receive_payload(DataPacket* packet);
    void test_registers();
};

#endif // NRF24_H
