#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>

#define SPI_OP (SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8) | SPI_LINES_SINGLE)
/* SPI Configuration */
const struct device *spi_dev;
struct spi_config spi_cfg = {
    .frequency = 1000000,                     // SPI frequency (1 MHz)
    .operation = SPI_OP,                     // SPI operation flags
    .slave = 0,                              // Slave ID (if applicable)
    .cs = NULL,                              // No chip select control (managed manually)
};

// Buffers for SPI communication
uint8_t tx_buffer[4] = {0xAA, 0xBB, 0xCC, 0xDD}; // Data to send to slave
uint8_t rx_buffer[4]; // Buffer to store received data

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

int main(void) {
    printk("SPI Master Test\n");

    /* Initialize SPI */
    spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi1)); // Adjust to match your hardware setup
    if (!device_is_ready(spi_dev)) {
        printk("SPI device not ready\n");
        return -1;
    }

    while (1) {
        // Use the corrected spi_transceive function
        int ret = spi_transceive(spi_dev, &spi_cfg, &tx_bufs, &rx_bufs);
        if (ret) {
            printk("SPI transceive failed: %d\n", ret);
        } else {
            printk("SPI transceive successful. Received: ");
            for (int i = 0; i < sizeof(rx_buffer); i++) {
                printk("0x%02X ", rx_buffer[i]);
            }
            printk("\n");
        }

        k_sleep(K_SECONDS(1)); // Delay between transactions
    }

    return 0;
}
