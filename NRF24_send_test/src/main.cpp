#include <zephyr/kernel.h>
#include "nrf24.h"

const struct device *spi_dev;


int main(void) {
    printk("nRF24L01+ Transmitter\n");

    spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi1));
    if (!device_is_ready(spi_dev)) {
        printk("SPI device not ready\n");
        return -1;
    }

    if (nrf24l01_init(spi_dev) != 0) {
        printk("Failed to initialize nRF24L01+\n");
        return -1;
    }

    struct DataPacket packet{-70, 60, 1};
    uint8_t data[sizeof(packet)];
    memcpy(data, &packet, sizeof(packet));

    //uint8_t data[4] = {'H', 'I', 'I', 'I'}; // Prepare data to send
    nrf24l01_test_registers();

    while (1) {
        // Prepare your payload data

        // Send payload
        if (nrf24l01_send_payload(data, sizeof(data)) != 0) {
            printk("Failed to send payload\n");
        } else {
            printk("Payload sent\n");
        }

        k_sleep(K_MSEC(1000)); // Wait 1 second before sending again
    }
    return 0;
}
