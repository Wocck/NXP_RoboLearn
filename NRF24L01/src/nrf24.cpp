// nrf24.cpp

#include "nrf24.h"

/* External Variables */
extern const struct device *spi_dev;
extern const struct device *gpio_dev;

/* Functions */

int nrf24l01_write_register(uint8_t reg, const uint8_t *buf, size_t len) {
    uint8_t cmd = 0x20 | (reg & 0x1F);
    uint8_t tx_buf[1 + len];

    tx_buf[0] = cmd;
    memcpy(&tx_buf[1], buf, len);

    struct spi_buf tx_bufs[] = {
        {
            .buf = tx_buf,
            .len = sizeof(tx_buf),
        },
    };
    struct spi_buf_set tx = {
        .buffers = tx_bufs,
        .count = ARRAY_SIZE(tx_bufs),
    };

    return spi_write(spi_dev, &spi_cfg, &tx);
}


int nrf24l01_read_register(uint8_t reg, uint8_t *buf, size_t len) {
    uint8_t cmd = reg & 0x1F;
    uint8_t tx_buf[1 + len];
    uint8_t rx_buf[1 + len];

    tx_buf[0] = cmd;
    memset(&tx_buf[1], 0xFF, len);

    struct spi_buf tx_bufs[] = {
        {
            .buf = tx_buf,
            .len = sizeof(tx_buf),
        },
    };
    struct spi_buf rx_bufs[] = {
        {
            .buf = rx_buf,
            .len = sizeof(rx_buf),
        },
    };
    struct spi_buf_set tx = {
        .buffers = tx_bufs,
        .count = ARRAY_SIZE(tx_bufs),
    };
    struct spi_buf_set rx = {
        .buffers = rx_bufs,
        .count = ARRAY_SIZE(rx_bufs),
    };

    int ret = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);
    if (ret != 0) {
        printk("SPI transceive failed: %d\n", ret);
    } else {
        memcpy(buf, &rx_buf[1], len);
    }

    return ret;
}


int nrf24l01_init(void) {
    /* Set the address width to 5 bytes (SETUP_AW) */
    uint8_t setup_aw = 0x03; // 5 bytes
    if (nrf24l01_write_register(0x03, &setup_aw, 1) != 0) {
        return -1;
    }

    /* Set the RX address for pipe 0 (RX_ADDR_P0) */
    uint8_t rx_address[5] = {'1', '0', '0', '0', '0'};
    if (nrf24l01_write_register(0x0A, rx_address, 5) != 0) {
        return -1;
    }

    /* Enable data pipe 0 (EN_RXADDR) */
    uint8_t en_rxaddr = 0x01; // Enable pipe 0
    if (nrf24l01_write_register(0x02, &en_rxaddr, 1) != 0) {
        return -1;
    }

    /* Set RF channel to 76 (RF_CH) */
    uint8_t rf_ch = 76; // Default channel used in RF24 library
    if (nrf24l01_write_register(0x05, &rf_ch, 1) != 0) {
        return -1;
    }

    /* Set RF_SETUP register to set data rate and power */
    uint8_t rf_setup = 0x26; // 250kbps, 0dBm power
    if (nrf24l01_write_register(0x06, &rf_setup, 1) != 0) {
        return -1;
    }

    /* Configure CONFIG register */
    uint8_t config = 0x0B; // Mask interrupts, PWR_UP, PRIM_RX
    if (nrf24l01_write_register(0x00, &config, 1) != 0) {
        return -1;
    }

    /* Delay for the module to settle */
    k_sleep(K_MSEC(100));

    return 0;
}

int nrf24l01_receive_payload(uint8_t *buf, size_t len) {
    uint8_t status;

    /* Read STATUS register */
    if (nrf24l01_read_register(0x07, &status, 1) != 0) {
        return -1;
    }

    if (status & 0x40) { // RX_DR flag is set
        /* Read payload */
        uint8_t cmd = 0x61; // R_RX_PAYLOAD
        uint8_t tx_buf[33];
        uint8_t rx_buf[33];

        tx_buf[0] = cmd;
        memset(&tx_buf[1], 0xFF, len);

        struct spi_buf tx_bufs[1];
        struct spi_buf rx_bufs[1];
        struct spi_buf_set tx;
        struct spi_buf_set rx;

        tx_bufs[0].buf = tx_buf;
        tx_bufs[0].len = 1 + len;

        rx_bufs[0].buf = rx_buf;
        rx_bufs[0].len = 1 + len;

        tx.buffers = tx_bufs;
        tx.count = 1;

        rx.buffers = rx_bufs;
        rx.count = 1;

        int ret = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);

        if (ret != 0) {
            return -1;
        }

        /* Copy received data */
        memcpy(buf, &rx_buf[1], len);

        /* Clear RX_DR flag */
        uint8_t clear = 0x40;
        if (nrf24l01_write_register(0x07, &clear, 1) != 0) {
            return -1;
        }

        return 0;
    }

    return -1; // No data received
}

void nrf24l01_test_registers(void) {
    uint8_t value;

    // Odczyt rejestru CONFIG
    if (nrf24l01_read_register(0x00, &value, 1) == 0) {
        printk("CONFIG register: 0x%02X\n", value);
    } else {
        printk("Failed to read CONFIG register\n");
    }

    // Odczyt rejestru RF_CH (kanał)
    if (nrf24l01_read_register(0x05, &value, 1) == 0) {
        printk("RF_CH register: 0x%02X\n", value);
    } else {
        printk("Failed to read RF_CH register\n");
    }

    // Odczyt rejestru RF_SETUP
    if (nrf24l01_read_register(0x06, &value, 1) == 0) {
        printk("RF_SETUP register: 0x%02X\n", value);
    } else {
        printk("Failed to read RF_SETUP register\n");
    }

    // Odczyt rejestru STATUS
    if (nrf24l01_read_register(0x07, &value, 1) == 0) {
        printk("STATUS register: 0x%02X\n", value);
    } else {
        printk("Failed to read STATUS register\n");
    }
}