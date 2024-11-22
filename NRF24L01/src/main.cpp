// main.cpp

#include "nrf24.h"

const struct device *spi_dev;
const struct device *gpio_dev;

struct spi_cs_control cs_ctrl;
struct spi_config spi_cfg;

int main(void) {
    printk("nRF24L01+ Receiver\n");

    /* Initialize SPI */
    spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi1));
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
    int ret = gpio_pin_configure(gpio_dev, CE_GPIO_PIN, GPIO_OUTPUT_LOW);
    if (ret != 0) {
        printk("Error configuring CE GPIO: %d\n", ret);
        return -1;
    }

    /* Configure CSN GPIO */
    ret = gpio_pin_configure(gpio_dev, CSN_GPIO_PIN, GPIO_OUTPUT_HIGH);
    if (ret != 0) {
        printk("Error configuring CSN GPIO: %d\n", ret);
        return -1;
    }

    /* Configure CSN GPIO for SPI CS control */
    cs_ctrl.gpio.port = gpio_dev;
    cs_ctrl.gpio.pin = CSN_GPIO_PIN;
    cs_ctrl.gpio.dt_flags = GPIO_ACTIVE_LOW; // Adjust based on your hardware
    cs_ctrl.delay = 0U; // No delay

    /* Configure SPI */
    spi_cfg.frequency = 1000000U;
    spi_cfg.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_OP_MODE_MASTER;
    spi_cfg.slave = 0;  // SPI slave number (usually 0)
    spi_cfg.cs = cs_ctrl; // Assign cs_ctrl to spi_cfg.cs

    if (!device_is_ready(spi_dev)) {
        printk("SPI device not ready\n");
        return -1;
    }

    /* Initialize nRF24L01+ */
    if (nrf24l01_init() != 0) {
        printk("Failed to initialize nRF24L01+\n");
        return -1;
    }

    uint8_t rx_buffer[32];

    nrf24l01_test_registers();

    while (1) {
        uint8_t status;
        if (nrf24l01_read_register(0x07, &status, 1) == 0) {
            printk("STATUS register: 0x%02X\n", status);
        }
        if (nrf24l01_receive_payload(rx_buffer, 5) == 0) {
            /* Process data */
            struct DataPacket {
                int16_t joystickX;
                int16_t joystickY;
                uint8_t buttonPressed;
            };

            struct DataPacket *data = (struct DataPacket *)rx_buffer;

            printk("Joystick X: %d\n", data->joystickX);
            printk("Joystick Y: %d\n", data->joystickY);
            printk("Button Pressed: %d\n", data->buttonPressed);
        }

        k_sleep(K_MSEC(100));
    }
    return 0;
}
