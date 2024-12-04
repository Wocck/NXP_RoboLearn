#ifndef NRF24_H
#define NRF24_H

#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>

#define CE_GPIO_PIN 2 ///< Pin for Chip Enable (CE), used to control RX/TX modes. GPIO_AD_B0_02 (D9)
#define CSN_GPIO_PIN 13 ///< Pin for Chip Select Not (CSN), used to control SPI communication. GPIO_SD_B0_01 (D10)
#define IRQ_GPIO_PIN 3 ///< Pin for Interrupt Request (IRQ), used to signal data received. GPIO_AD_B0_03 (D8)

struct DataPacket {
    int8_t joystickX; ///< X-axis value of the joystick
    int8_t joystickY; ///< Y-axis value of the joystick
    uint8_t buttonPressed; ///< Button pressed status (0 - not pressed, 1 - pressed)
} __attribute__((packed));


class NRF24 {
private:
    const struct device* gpio_dev_1; ///< GPIO device for CE pin
    const struct device* spi_dev; ///< SPI device for communication
    struct spi_config spi_cfg; ///< SPI configuration
    uint8_t tx_buf[33]; ///< Transmit buffer for SPI communication
    uint8_t rx_buf[33]; ///< Receive buffer for SPI communication

public:
    NRF24(const struct device* spi);
    int init();
    void reset(uint8_t reg);
    int set_device(const struct device* spi);

    int write_register(uint8_t reg, const uint8_t* data, size_t len);
    int read_register(uint8_t reg, uint8_t* data, size_t len);

    int receive_payload(DataPacket* packet);
    void test_registers();
};

#endif // NRF24_H