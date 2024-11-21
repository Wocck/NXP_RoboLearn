// nrf24.cpp

#include "nrf24.h"

/* External Variables */
extern const struct device *spi_dev;
extern const struct device *gpio_dev;

/* Functions */

int nrf24l01_write_register(uint8_t reg, const uint8_t *buf, size_t len) {
    uint8_t cmd = 0x20 | (reg & 0x1F);
    struct spi_buf tx_bufs[2];
    struct spi_buf_set tx;

    tx_bufs[0].buf = &cmd;
    tx_bufs[0].len = 1;
    tx_bufs[1].buf = (uint8_t *)buf;
    tx_bufs[1].len = len;

    tx.buffers = tx_bufs;
    tx.count = 2;

    return spi_write(spi_dev, &spi_cfg, &tx);
}

int nrf24l01_read_register(uint8_t reg, uint8_t *buf, size_t len) {
    uint8_t cmd = reg & 0x1F;
    struct spi_buf tx_bufs[1];
    struct spi_buf rx_bufs[2];
    struct spi_buf_set tx;
    struct spi_buf_set rx;

    tx_bufs[0].buf = &cmd;
    tx_bufs[0].len = 1;

    rx_bufs[0].buf = NULL;
    rx_bufs[0].len = 1;
    rx_bufs[1].buf = buf;
    rx_bufs[1].len = len;

    tx.buffers = tx_bufs;
    tx.count = 1;

    rx.buffers = rx_bufs;
    rx.count = 2;

    return spi_transceive(spi_dev, &spi_cfg, &tx, &rx);
}

int nrf24l01_init(void) {
    /* Set CE low */
    gpio_pin_set(gpio_dev, CE_GPIO_PIN, 0);

    /* Configure nRF24L01+ registers */
    uint8_t config = 0x0B; // Mask interrupts, PWR_UP, PRIM_RX
    if (nrf24l01_write_register(0x00, &config, 1) != 0) {
        return -1;
    }

    /* Additional configuration can be added here */

    /* Set CE high to enter RX mode */
    gpio_pin_set(gpio_dev, CE_GPIO_PIN, 1);

    return 0;
}

int nrf24l01_receive_payload(uint8_t *buf, size_t len) {
    uint8_t status;
    if (nrf24l01_read_register(0x07, &status, 1) != 0) {
        return -1;
    }

    if (status & 0x40) { // RX_DR flag
        uint8_t cmd = 0x61; // R_RX_PAYLOAD
        struct spi_buf tx_bufs[1];
        struct spi_buf rx_bufs[2];
        struct spi_buf_set tx;
        struct spi_buf_set rx;

        tx_bufs[0].buf = &cmd;
        tx_bufs[0].len = 1;

        rx_bufs[0].buf = NULL;
        rx_bufs[0].len = 1;
        rx_bufs[1].buf = buf;
        rx_bufs[1].len = len;

        tx.buffers = tx_bufs;
        tx.count = 1;

        rx.buffers = rx_bufs;
        rx.count = 2;

        if (spi_transceive(spi_dev, &spi_cfg, &tx, &rx) != 0) {
            return -1;
        }

        /* Clear RX_DR flag */
        uint8_t clear = 0x40;
        if (nrf24l01_write_register(0x07, &clear, 1) != 0) {
            return -1;
        }

        return 0;
    }

    return -1; // No data
}
