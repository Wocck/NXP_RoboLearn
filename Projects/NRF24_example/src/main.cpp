#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "nrf24.h"

static struct gpio_callback irq_callback_data;
const struct device* spi_dev = DEVICE_DT_GET(DT_NODELABEL(lpspi1));
static NRF24 radio(spi_dev);
static struct k_work work; // Work queue for processing interrupt

// Work handler to process the interrupt
void process_irq_work(struct k_work *work) {
    printk("Processing IRQ in work queue\n");

    while (true) {
        uint8_t fifo_status;
        if (radio.read_register(0x17, &fifo_status, 1) != 0) { // FIFO_STATUS register
            printk("Failed to read FIFO_STATUS register\n");
            break;
        }

        if (fifo_status & 0x01) { // RX_EMPTY bit
            break; // FIFO RX is empty, exit
        }

        struct DataPacket packet;
        if (radio.receive_payload(&packet) == 0) {
            printk("Received packet: joystickX=%d, joystickY=%d, buttonPressed=%d\n",
                   packet.joystickX, packet.joystickY, packet.buttonPressed);
        } else {
            printk("Failed to receive payload\n");
        }

        // Clear RX_DR flag
        uint8_t clear_flags = 0x40;
        radio.write_register(0x07, &clear_flags, 1);
    }
}

// Interrupt handler
static void irq_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    printk("IRQ pin triggered!\n");

    // Schedule the work to process the interrupt
    k_work_submit(&work);
}

int main(void) {
    printk("nRF24L01+ Receiver\n");

    const struct device* irq_dev = DEVICE_DT_GET(DT_NODELABEL(gpio1));
    int ret = gpio_pin_configure(irq_dev, IRQ_GPIO_PIN, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Failed to configure IRQ pin\n");
        return ret;
    }

    ret = gpio_pin_interrupt_configure(irq_dev, IRQ_GPIO_PIN, GPIO_INT_EDGE_TO_INACTIVE);
    if (ret < 0) {
        printk("Failed to configure IRQ pin interrupts\n");
        return ret;
    }

    gpio_init_callback(&irq_callback_data, irq_handler, BIT(IRQ_GPIO_PIN));
    gpio_add_callback(irq_dev, &irq_callback_data);

    printk("IRQ pin interrupt configured\n");

    if (!device_is_ready(spi_dev)) {
        printk("SPI device not ready\n");
        return -1;
    }

    radio.reset(0);

    if (radio.init() != 0) {
        printk("Failed to initialize nRF24L01+\n");
        return -1;
    }

    radio.test_registers();

    // Initialize work queue
    k_work_init(&work, process_irq_work);

    while (1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
