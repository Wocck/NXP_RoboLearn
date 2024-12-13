#include <zephyr/kernel.h>
#include "engine.h"
#include "nrf24.h"

int main(void) {
    printk("Starting remote-controlled robot\n");

    // Initialize SPI device for nRF24
    const struct device* spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi1));
    if (!device_is_ready(spi_dev)) {
        printk("SPI device not ready\n");
        return -1;
    }

    const struct device* gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));
    if (!device_is_ready(gpio_dev)) {
        printk("GPIO1 device not ready\n");
        return -1;
    }

    // Initialize NRF24 and Engine with shared GPIO device
    NRF24 radio(gpio_dev, spi_dev);
    Engine engine(gpio_dev);

    //Reset and initialize nRF24L01+ module
    radio.reset(0);
    if (radio.init() != 0) {
        printk("Failed to initialize nRF24L01+\n");
        return -1;
    }

    // Configure IRQ for the nRF24 module
    if (radio.configure_irq() != 0) {
        printk("Failed to configure IRQ\n");
        return -1;
    }

    // Initialize the engine (PWM and GPIO setup)
    if (!engine.init()) {
        printk("Failed to initialize engine\n");
        return -1;
    }

    radio.test_registers();

    while(!radio.is_receiving()){
        k_sleep(K_MSEC(100));
    }

    // Main loop
    while (1) {
        // Get the latest DataPacket from the nRF24 module
        DataPacket packet = radio.get_current_packet();

        // Log received data
        // printk("Received: joystickX=%d, joystickY=%d, buttonPressed=%d\n",
        //        packet.joystickX, packet.joystickY, packet.buttonPressed);

        // Control motors based on received packet
        engine.controlMotors(packet);

        // Small delay
        k_sleep(K_MSEC(50));
    }

    return 0;
}
