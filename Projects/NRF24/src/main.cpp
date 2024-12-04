#include <zephyr/kernel.h>
#include "nrf24.h"

int main(void) {
    printk("nRF24L01+ Receiver with IRQ\n");

    // Initialize SPI device
    const struct device* spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi1));
    if (!device_is_ready(spi_dev)) {
        printk("SPI device not ready\n");
        return -1;
    }

    // Create NRF24 object
    NRF24 radio(spi_dev);

    // Reset and initialize nRF24L01+ module
    radio.reset(0);
    if (radio.init() != 0) {
        printk("Failed to initialize nRF24L01+\n");
        return -1;
    }

    // Configure IRQ for the module
    if (radio.configure_irq() != 0) {
        printk("Failed to configure IRQ\n");
        return -1;
    }

    // Main loop
    while (1) {
        k_sleep(K_SECONDS(1));
        DataPacket packet = radio.get_current_packet();
        printk("Current packet: joystickX=%d, joystickY=%d, buttonPressed=%d\n",
               packet.joystickX, packet.joystickY, packet.buttonPressed);
    }

    return 0;
}