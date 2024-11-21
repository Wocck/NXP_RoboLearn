// nrf24.h

#ifndef NRF24L01_H
#define NRF24L01_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

/* SPI Device */
#define SPI_DEV DT_NODELABEL(lpspi3)

/* GPIO Device */
#define GPIO_DEV DT_NODELABEL(gpio1)

/* CE GPIO */
#define CE_GPIO_PIN    4  // Replace with your actual CE pin number
#define CE_GPIO_FLAGS  GPIO_OUTPUT_INIT_HIGH

/* CSN GPIO */
#define CSN_GPIO_PIN   3  // Replace with your actual CSN pin number
#define CSN_GPIO_FLAGS GPIO_OUTPUT_INIT_HIGH

/* Global Variables */
extern const struct device *spi_dev;
extern const struct device *gpio_dev;

/* SPI Configuration */
extern struct spi_cs_control cs_ctrl;
extern struct spi_config spi_cfg;

/* Function Declarations */
int nrf24l01_write_register(uint8_t reg, const uint8_t *buf, size_t len);
int nrf24l01_read_register(uint8_t reg, uint8_t *buf, size_t len);
int nrf24l01_init(void);
int nrf24l01_receive_payload(uint8_t *buf, size_t len);

#endif /* NRF24L01_H */
