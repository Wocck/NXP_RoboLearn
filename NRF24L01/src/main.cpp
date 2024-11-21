// main.cpp

#include "nrf24.h"

const struct device *spi_dev;
const struct device *gpio_dev;

struct spi_cs_control cs_ctrl;
struct spi_config spi_cfg;

int main(void) {
    printk("nRF24L01+ Receiver\n");

    /* Initialize SPI */
    spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi3));
    if (!spi_dev) {
        printk("Cannot find SPI device\n");
        return -1;
    }

    /* Initialize GPIO */
    gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));
    if (!gpio_dev) {
        printk("Cannot find GPIO device\n");
        return -1;
    }

   /* Configure CE GPIO */
    int ret = gpio_pin_configure(gpio_dev, CE_GPIO_PIN, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Error configuring CE GPIO: %d\n", ret);
        return -1;
    }

    /* Configure CSN GPIO */
    ret = gpio_pin_configure(gpio_dev, CSN_GPIO_PIN, GPIO_OUTPUT_ACTIVE);
    if (ret != 0) {
        printk("Error configuring CSN GPIO: %d\n", ret);
        return -1;
    }

    /* Configure SPI */
    spi_cfg.frequency = 8000000U;
    spi_cfg.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER;
    spi_cfg.slave = 0;  // SPI slave number (usually 0)
    spi_cfg.cs = cs_ctrl;

    /* Configure CS Control for SPI */
    cs_ctrl.gpio.port = gpio_dev;
    cs_ctrl.gpio.pin = CSN_GPIO_PIN;
    cs_ctrl.gpio.dt_flags = GPIO_ACTIVE_HIGH;
    cs_ctrl.delay = 0U;

    /* Initialize nRF24L01+ */
    if (nrf24l01_init() != 0) {
        printk("Failed to initialize nRF24L01+\n");
        return -1;
    }

    uint8_t rx_buffer[32];

    while (1) {
        /* Receive data */
        if (nrf24l01_receive_payload(rx_buffer, sizeof(rx_buffer)) == 0) {
            /* Process data */
            printk("Received data: ");
            for (size_t i = 0; i < sizeof(rx_buffer); i++) {
                printk("%02X ", rx_buffer[i]);
            }
            printk("\n");
        }

        k_sleep(K_MSEC(100));
    }
    return 0;
}
