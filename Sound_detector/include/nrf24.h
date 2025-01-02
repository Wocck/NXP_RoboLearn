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
#include <stdio.h>
#include <zephyr/drivers/gpio.h>
#include "datapacket.h"

#define CE_GPIO_PIN 2 
#define CSN_GPIO_PIN 13
#define IRQ_GPIO_PIN 3


class NRF24 {
private:
    const struct device* gpio_dev_1;
    const struct device* spi_dev;
    struct spi_config spi_cfg;
    const struct device* gpio_dev_irq; 
    struct gpio_callback irq_callback; 
    struct k_work irq_work; 
    uint8_t tx_buf[33];
    uint8_t rx_buf[33];

    DataPacket current_packet; 
    bool received_data=false;

    
    static void irq_handler(const struct device* dev, struct gpio_callback* cb, uint32_t pins);
    static void process_irq_work(struct k_work* work);
    void handle_irq();
    int set_device(const struct device* spi);
    int write_register(uint8_t reg, const uint8_t* data, size_t len);
    int read_register(uint8_t reg, uint8_t* data, size_t len);
    int send_command(uint8_t command, uint8_t* response, size_t response_len);

public:

    NRF24(const struct device* gpio, const struct device* spi);
    void reset(uint8_t reg);
    int init();
    int receive_payload(DataPacket* packet);
    int configure_irq();
    void test_registers();
    void log_register(uint8_t reg);
    DataPacket get_current_packet();
    bool is_receiving() { return received_data; }
    int send_ack_payload(const char* message);
    int send_ack_payload(const uint8_t* data, size_t length);
};

#endif // NRF24_H
