#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

/* SPI configuration */
const struct device *spi_dev;
struct spi_cs_control cs_ctrl;
struct spi_config spi_cfg;

/* GPIO for CS control */
const struct device *gpio_dev;

#define CSN_GPIO_PIN 10 // Ustaw numer pinu CSN zgodnie z Twoją konfiguracją

void spi_test_communication(void) {
    uint8_t tx_buf[] = "Hello ESP32";
    uint8_t rx_buf[sizeof(tx_buf)];
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
        .count = 1,
    };
    struct spi_buf_set rx = {
        .buffers = rx_bufs,
        .count = 1,
    };

    int ret = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);
    if (ret == 0) {
        printk("SPI transaction successful.\n");
        printk("Received data: ");
        for (size_t i = 0; i < sizeof(rx_buf); i++) {
            printk("%c", rx_buf[i]);
        }
        printk("\n");
    } else {
        printk("SPI transaction failed: %d\n", ret);
    }
}

int main(void) {
    printk("SPI Master Test\n");

    /* Initialize SPI */
    spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi1)); // Adjust as needed
    if (!device_is_ready(spi_dev)) {
        printk("SPI device not ready\n");
        return -1;
    }

    /* Initialize GPIO for CS */
    gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));
    if (!device_is_ready(gpio_dev)) {
        printk("GPIO device not ready\n");
        return -1;
    }

    /* Configure CS GPIO */
    int ret = gpio_pin_configure(gpio_dev, CSN_GPIO_PIN, GPIO_OUTPUT_HIGH);
    if (ret != 0) {
        printk("Error configuring CS GPIO: %d\n", ret);
        return -1;
    }

    /* Configure CSN for SPI CS control */
    cs_ctrl.gpio.port = gpio_dev;
    cs_ctrl.gpio.pin = CSN_GPIO_PIN;
    cs_ctrl.gpio.dt_flags = GPIO_ACTIVE_LOW;
    cs_ctrl.delay = 0U;

    /* Configure SPI */
    spi_cfg.frequency = 1000000U; // 1 MHz
    spi_cfg.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER;
    spi_cfg.slave = 0;
    spi_cfg.cs = cs_ctrl; // Użyj adresu struktury cs_ctrl

    while (1) {
        spi_test_communication();
        k_sleep(K_SECONDS(1));
    }
    return 0;
}
