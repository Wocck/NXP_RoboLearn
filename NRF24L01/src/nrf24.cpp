#include "nrf24.h"
#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>

#define CE_GPIO_PIN 2 // GPIO_AD_B0_03 (D9)
#define CSN_GPIO_PIN 13 // GPIO_SD_B0_01 (D10)

// Rejestry nRF24L01
#define CONFIG_REG 0x00
#define STATUS_REG 0x07
#define RX_ADDR_P0 0x0A
#define SETUP_AW 0x03
#define RF_CH 0x05
#define RF_SETUP 0x06
#define EN_RXADDR 0x02
#define RX_PW_P0 0x11
#define R_RX_PAYLOAD 0x61
#define FLUSH_RX 0xE2
#define EN_AA 0x01

static const struct device *gpio_dev_1;
static const struct device *gpio_dev_3;
static const struct device *spi_dev;

uint8_t tx_buf[33];
uint8_t rx_buf[33];

static struct spi_config spi_cfg = {
    .frequency = 100000,
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE | SPI_HOLD_ON_CS,
    .slave = 0U,
    .cs = {
        .gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(lpspi1), cs_gpios), // Correct GPIO spec
        .delay = 0U, // No delay
    },
    //.cs = NULL,
};

static int nrf24l01_write_register(uint8_t reg, const uint8_t *data, size_t len) {
    uint8_t cmd = 0x20 | (reg & 0x1F); // Write command
    tx_buf[0] = cmd;
    memcpy(&tx_buf[1], data, len);

    struct spi_buf spi_tx = { .buf = tx_buf, .len = len + 1 };
    struct spi_buf spi_rx = { .buf = rx_buf, .len = len + 1 };
    
    struct spi_buf_set tx = { .buffers = &spi_tx, .count = 1 };
    struct spi_buf_set rx = { .buffers = &spi_rx, .count = 1 };

    //gpio_pin_set(gpio_dev_3, CSN_GPIO_PIN, 0);
    int ret = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);
    //gpio_pin_set(gpio_dev_3, CSN_GPIO_PIN, 1);
    
    return ret;
}

static int nrf24l01_read_register(uint8_t reg, uint8_t *data, size_t len) {
    uint8_t cmd = reg & 0x1F; // Read command
    tx_buf[0] = cmd;
    memset(&tx_buf[1], 0xFF, len);

    struct spi_buf spi_tx = { .buf = tx_buf, .len = len + 1 };
    struct spi_buf spi_rx = { .buf = rx_buf, .len = len + 1 };

    struct spi_buf_set tx = { .buffers = &spi_tx, .count = 1 };
    struct spi_buf_set rx = { .buffers = &spi_rx, .count = 1 };


    int ret = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);

    if (ret == 0) {
        memcpy(data, &rx_buf[1], len); // Omit the command byte
    }
    return ret;
}

static int nrf24l01_read_multi_register(uint8_t reg, uint8_t *data, size_t len) {
    uint8_t cmd = reg & 0x1F; // Read command

    uint8_t tx_buffer[33] = {0};
    tx_buffer[0] = cmd;
    uint8_t rx_buffer[33] = {0};
    struct spi_buf tx_buf = {
    .buf = tx_buffer,
    .len = sizeof(tx_buffer),
    };

    struct spi_buf rx_buf = {
        .buf = rx_buffer,
        .len = sizeof(rx_buffer),
    };

    struct spi_buf_set tx_bufs = {
        .buffers = &tx_buf,
        .count = 1,
    };

    struct spi_buf_set rx_bufs = {
        .buffers = &rx_buf,
        .count = 1,
    };

    int ret = spi_transceive(spi_dev, &spi_cfg, &tx_bufs, &rx_bufs);

    if (ret == 0) {
    } else {
        printk("SPI Read Error: %d\n", ret);
    }
    return ret;
}

int nrf24l01_init(const struct device *spi) {
    spi_dev = spi;
    gpio_dev_1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));
    gpio_dev_3 = DEVICE_DT_GET(DT_NODELABEL(gpio3));
    
    if (!gpio_dev_1) {
        printk("GPIO_1 device not ready\n");
        return -1;
    }

    if (!gpio_dev_3) {
        printk("GPIO_3 device not ready\n");
        return -1;
    }

    // Configure CE and CSN pins
    gpio_pin_configure(gpio_dev_1, CE_GPIO_PIN, GPIO_OUTPUT_LOW);
    //gpio_pin_configure(gpio_dev_3, CSN_GPIO_PIN, GPIO_OUTPUT_HIGH);
    k_sleep(K_MSEC(10));

    // Initialize nRF24L01
    uint8_t config = 0x0B; // Power up, PRIM_RX
    if (nrf24l01_write_register(CONFIG_REG, &config, 1) != 0) {
        printk("Failed to write CONFIG register\n");
        return -1;
    }

    uint8_t rf_addr_width = 0x03;
    if (nrf24l01_write_register(SETUP_AW, &rf_addr_width, 1) != 0) {
        printk("Failed to set Addresses Width\n");
        return -1;
    }

    uint8_t en_rxaddr = 0x01; // Enable pipe 0
    if (nrf24l01_write_register(EN_RXADDR, &en_rxaddr, 1) != 0) {
        printk("Failed to enable RX pipe\n");
        return -1;
    }

    uint8_t rx_addr[5] = {0x11, 0x22, 0x33, 0x44, 0x55}; // Pipe Address
    if (nrf24l01_write_register(RX_ADDR_P0, rx_addr, 5) != 0) {
        printk("Failed to set RX address\n");
        return -1;
    }
    k_sleep(K_MSEC(10));

    uint8_t payload_size = 32; // Payload size
    if (nrf24l01_write_register(RX_PW_P0, &payload_size, 1) != 0) {
        printk("Failed to set payload size\n");
        return -1;
    }

    uint8_t rf_ch = 76; // Channel
    if (nrf24l01_write_register(RF_CH, &rf_ch, 1) != 0) {
        printk("Failed to set RF channel\n");
        return -1;
    }

    uint8_t rf_setup = 0x25; // 250kbps, 0dBm
    if (nrf24l01_write_register(RF_SETUP, &rf_setup, 1) != 0) {
        printk("Failed to set RF setup\n");
        return -1;
    }

    uint8_t en_aa = 0x00; // Disable Auto Acknowledgment
    if (nrf24l01_write_register(EN_AA, &en_aa, 1) != 0) {
        printk("Failed to disable Auto Acknowledgment\n");
        return -1;
    };


    gpio_pin_set(gpio_dev_1, CE_GPIO_PIN, 1); // Enable receiver
    k_sleep(K_MSEC(10));
    return 0;
}

int nrf24l01_receive_payload(struct DataPacket *packet) {
    uint8_t status;
    if (nrf24l01_read_register(STATUS_REG, &status, 1) != 0) {
        return -1;
    }
    //printk("Status: %d\n\r", status);

    if (status & 0x40) { // RX_DR flag
        uint8_t cmd = R_RX_PAYLOAD;
        struct spi_buf tx_buf = { .buf = &cmd, .len = 1 };
        struct spi_buf rx_buf = { .buf = packet, .len = sizeof(struct DataPacket) };

        struct spi_buf_set tx = { .buffers = &tx_buf, .count = 1 };
        struct spi_buf_set rx = { .buffers = &rx_buf, .count = 1 };

        int ret = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);
        if (ret == 0) {
            // Clear RX_DR flag
            uint8_t clear = 0x40;
            nrf24l01_write_register(STATUS_REG, &clear, 1);
        }
        return ret;
    }
    return -1;
}

void nrf24l01_test_registers(void) {
    uint8_t value;
    printk("Testing nRF24L01 Registers...\n");

    // Read CONFIG register
    if (nrf24l01_read_register(CONFIG_REG, &value, 1) == 0) {
        printk("CONFIG register (0x00): 0x%02X\n", value);
    } else {
        printk("Failed to read CONFIG register\n");
    }

    // Read EN_RXADDR register
    if (nrf24l01_read_register(EN_RXADDR, &value, 1) == 0) {
        printk("EN_RXADDR register (0x02): 0x%02X\n", value);
    } else {
        printk("Failed to read EN_RXADDR register\n");
    }

    // Read RF_CH register
    if (nrf24l01_read_register(RF_CH, &value, 1) == 0) {
        printk("RF_CH register (0x05): 0x%02X\n", value);
    } else {
        printk("Failed to read RF_CH register\n");
    }

    // Read RF_SETUP register
    if (nrf24l01_read_register(RF_SETUP, &value, 1) == 0) {
        printk("RF_SETUP register (0x06): 0x%02X\n", value);
    } else {
        printk("Failed to read RF_SETUP register\n");
    }

    // Read STATUS register
    if (nrf24l01_read_register(STATUS_REG, &value, 1) == 0) {
        printk("STATUS register (0x07): 0x%02X\n", value);
    } else {
        printk("Failed to read STATUS register\n");
    }

    // Read RX_ADDR_P0 register
    uint8_t rx_addr[5];
    if (nrf24l01_read_register(RX_ADDR_P0, rx_addr, 5) == 0) {
        printk("RX_ADDR_P0 register (0x0A): ");
        for (int i = 0; i < 5; i++) {
            printk("0x%02X ", rx_addr[i]);
        }
        printk("\n");
    } else {
        printk("Failed to read RX_ADDR_P0 register\n");
    }

    // Read RX_PW_P0 register
    if (nrf24l01_read_register(RX_PW_P0, &value, 1) == 0) {
        printk("RX_PW_P0 register (0x11): 0x%02X\n", value);
    } else {
        printk("Failed to read RX_PW_P0 register\n");
    }

    if (nrf24l01_read_register(SETUP_AW, &value, 1) == 0) {
        printk("Addr Width register (0x03): 0x%02X\n", value);
    } else {
        printk("Failed to read RX_PW_P0 register\n");
    }

    printk("Register test complete.\n");
}