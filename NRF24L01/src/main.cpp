#include <zephyr/kernel.h>
#include "nrf24.h"

const struct device *spi_dev;

int main(void) {
    printk("nRF24L01+ Receiver\n");

    spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi1));
    if (!device_is_ready(spi_dev)) {
        printk("SPI device not ready\n");
        return -1;
    }

    if (nrf24l01_init(spi_dev) != 0) {
        printk("Failed to initialize nRF24L01+\n");
        return -1;
    }

    struct DataPacket packet;
    while (1) {
        if (nrf24l01_receive_payload(&packet) == 0) {
            printk("Joystick X: %d\n", packet.joystickX);
            printk("Joystick Y: %d\n", packet.joystickY);
            printk("Button Pressed: %d\n", packet.buttonPressed);
        }
        k_sleep(K_MSEC(100));
    }
    return 0;
}
