/**
 * @file nrf24.h
 * @author Wojtek Sekula
 * @date 2024-12-01
 * @brief Header file for NRF24 class, providing functionality to interface with the nRF24L01+ module.
 *
 * This file contains the definition of the NRF24 class and related methods to
 * initialize, reset, and communicate with the nRF24L01+ module over SPI.
 */

#ifndef NRF24_H
#define NRF24_H

#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>

/**
 * @brief GPIO pin definitions for the nRF24L01+ module.
 */
#define CE_GPIO_PIN 2 ///< Pin for Chip Enable (CE), used to control RX/TX modes. GPIO_AD_B0_02 (D9)
#define CSN_GPIO_PIN 13 ///< Pin for Chip Select Not (CSN), used to control SPI communication. GPIO_SD_B0_01 (D10)
#define IRQ_GPIO_PIN 3 ///< Pin for Interrupt Request (IRQ), used to signal data received. GPIO_AD_B0_03 (D8)

/**
 * @struct DataPacket
 * @brief Data structure representing a packet sent/received via the nRF24L01+ module.
 */
struct DataPacket {
    int8_t joystickX; ///< X-axis value of the joystick
    int8_t joystickY; ///< Y-axis value of the joystick
    uint8_t buttonPressed; ///< Button pressed status (0 - not pressed, 1 - pressed)
} __attribute__((packed));

/**
 * @class NRF24
 * @brief A class to interface with the nRF24L01+ module for wireless communication.
 *
 * This class provides methods for initializing the module, resetting its registers,
 * testing registers, and receiving data packets.
 */
class NRF24 {
private:
    const struct device* gpio_dev_1; ///< GPIO device for CE pin
    const struct device* spi_dev; ///< SPI device for communication
    struct spi_config spi_cfg; ///< SPI configuration
    const struct device* gpio_dev_irq; ///< GPIO device for IRQ pin
    struct gpio_callback irq_callback; ///< GPIO callback structure for IRQ
    struct k_work irq_work; ///< Work queue structure for interrupt processing
    uint8_t tx_buf[33]; ///< Transmit buffer for SPI communication
    uint8_t rx_buf[33]; ///< Receive buffer for SPI communication

    DataPacket current_packet; ///< Przechowuje aktualny stan joysticka i przycisku

    /**
     * @brief Callback function to handle IRQ events.
     * @param dev Pointer to the GPIO device.
     * @param cb Pointer to the callback structure.
     * @param pins Bitmask of the pins that triggered the interrupt.
     */
    static void irq_handler(const struct device* dev, struct gpio_callback* cb, uint32_t pins);
    
    /**
     * @brief Work handler to process the interrupt.
     * @param work Pointer to the work structure.
     */
    static void process_irq_work(struct k_work* work);

    /**
     * @brief Internal function to process IRQ within the object.
     */
    void handle_irq();

    /**
     * @brief Set up the SPI and GPIO devices.
     * @param spi Pointer to the SPI device to use.
     * @return 0 on success, negative error code otherwise.
     */
    int set_device(const struct device* spi);

    /**
     * @brief Write data to a specified nRF24L01+ register.
     * @param reg Register address to write to.
     * @param data Pointer to the data buffer to write.
     * @param len Length of the data to write.
     * @return 0 on success, negative error code otherwise.
     */
    int write_register(uint8_t reg, const uint8_t* data, size_t len);

    /**
     * @brief Read data from a specified nRF24L01+ register.
     * @param reg Register address to read from.
     * @param data Pointer to the buffer to store read data.
     * @param len Length of the data to read.
     * @return 0 on success, negative error code otherwise.
     */
    int read_register(uint8_t reg, uint8_t* data, size_t len);

public:
     /**
     * @brief Constructor for the NRF24 class.
     * @param spi Pointer to the SPI device to use.
     */
    NRF24(const struct device* spi);

    /**
     * @brief Reset specific or all nRF24L01+ registers.
     * @param reg Register address to reset. Pass 0 to reset all registers.
     */
    void reset(uint8_t reg);

    /**
     * @brief Initialize the nRF24L01+ module.
     * @return 0 on success, negative error code otherwise.
     */
    int init();

    /**
     * @brief Receive a data packet from the nRF24L01+ module.
     * @param packet Pointer to the DataPacket structure to store received data.
     * @return 0 on success, negative error code otherwise.
     */
    int receive_payload(DataPacket* packet);

    /**
     * @brief Configure IRQ pin and enable interrupts.
     * @return 0 on success, negative error code otherwise.
     */
    int configure_irq();

    /**
     * @brief Test and print the values of nRF24L01+ registers.
     */
    void test_registers();

    /**
     * @brief Get the current DataPacket object.
     * @return Kopia aktualnych danych z joysticka i przycisku.
     */
    DataPacket get_current_packet() const;
};

#endif // NRF24_H
