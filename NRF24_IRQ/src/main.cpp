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

    // Initialize NRF24 object
    NRF24 radio = NRF24(spi_dev);

    // Initialize the nRF24L01+ module
    if (radio.init() != 0) {
        printk("Failed to initialize nRF24L01+\n");
        return -1;
    }

    // Test registers
    radio.test_registers();

    // Główna pętla
    while (1) {
        // Pobierz aktualny pakiet
        DataPacket packet = radio.get_current_packet();

        // Wyświetl dane pakietu
        // printk("Joystick Data:\n");
        // printk("    X: %d\n", packet.joystickX);
        // printk("    Y: %d\n", packet.joystickY);
        // printk("    Button: %d\n", packet.buttonPressed);

        // Odświeżanie co 100 ms
        k_sleep(K_MSEC(100));
    }

    return 0;
}
