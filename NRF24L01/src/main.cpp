#include <zephyr/kernel.h>
#include "nrf24.h"

int main(void) {
    printk("nRF24L01+ Receiver\n");

    // Initialize SPI device
    const struct device* spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi1));
    if (!device_is_ready(spi_dev)) {
        printk("SPI device not ready\n");
        return -1;
    }

    // Create NRF24 object
    NRF24 radio(spi_dev);

    // Initialize the nRF24L01+ module
    if (radio.init() != 0) {
        printk("Failed to initialize nRF24L01+\n");
        return -1;
    }

    // Test registers
    radio.test_registers();

    // Main loop to receive data packets
    struct DataPacket packet;
    while (1) {
        if (radio.receive_payload(&packet) == 0) {
            printk("Received Data Packet\n");
            printk("    Joystick X: %d\n", packet.joystickX);
            printk("    Joystick Y: %d\n", packet.joystickY);
            printk("    Button Pressed: %d\n\n", packet.buttonPressed);
        }
        k_sleep(K_MSEC(10));
    }

    return 0;
}
