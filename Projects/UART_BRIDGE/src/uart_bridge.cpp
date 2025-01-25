#include "uart_bridge.h"
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(uart_bridge, LOG_LEVEL_INF);

void uart_bridge_loop(const struct device *uart_in, const struct device *uart_out) {
    uint8_t buffer[1];

    while (1) {
        // Odczyt danych z wejściowego UART
        if (uart_poll_in(uart_in, buffer) == 0) {
            LOG_INF("Received: %c", buffer[0]);

            // Wysłanie danych na wyjściowy UART
            uart_poll_out(uart_out, buffer[0]);
        }
        
        // Sprawdzenie danych z wyjściowego UART (jeśli wymagane w drugą stronę)
        if (uart_poll_in(uart_out, buffer) == 0) {
            LOG_INF("Loopback: %c", buffer[0]);

            uart_poll_out(uart_in, buffer[0]);
        }

        k_msleep(10);
    }
}

void print_uart_config(const struct device *uart_dev) {
    struct uart_config uart_cfg;

    // Pobranie bieżącej konfiguracji UART
    int ret = uart_config_get(uart_dev, &uart_cfg);
    if (ret != 0) {
        printk("Failed to get UART configuration: %d\n", ret);
        return;
    }

    // Wypisanie konfiguracji w elegancki sposób
    printk("UART Configuration for %s:\n", uart_dev->name);
    printk("  Baudrate: %d\n", uart_cfg.baudrate);

    printk("  Parity: ");
    switch (uart_cfg.parity) {
    case UART_CFG_PARITY_NONE:
        printk("None\n");
        break;
    case UART_CFG_PARITY_EVEN:
        printk("Even\n");
        break;
    case UART_CFG_PARITY_ODD:
        printk("Odd\n");
        break;
    default:
        printk("Unknown\n");
        break;
    }

    printk("  Stop Bits: ");
    switch (uart_cfg.stop_bits) {
    case UART_CFG_STOP_BITS_1:
        printk("1\n");
        break;
    case UART_CFG_STOP_BITS_2:
        printk("2\n");
        break;
    default:
        printk("Unknown\n");
        break;
    }

    printk("  Data Bits: ");
    switch (uart_cfg.data_bits) {
    case UART_CFG_DATA_BITS_5:
        printk("5\n");
        break;
    case UART_CFG_DATA_BITS_6:
        printk("6\n");
        break;
    case UART_CFG_DATA_BITS_7:
        printk("7\n");
        break;
    case UART_CFG_DATA_BITS_8:
        printk("8\n");
        break;
    case UART_CFG_DATA_BITS_9:
        printk("9\n");
        break;
    default:
        printk("Unknown\n");
        break;
    }

    printk("  Flow Control: ");
    switch (uart_cfg.flow_ctrl) {
    case UART_CFG_FLOW_CTRL_NONE:
        printk("None\n");
        break;
    case UART_CFG_FLOW_CTRL_RTS_CTS:
        printk("RTS/CTS\n");
        break;
    default:
        printk("Unknown\n");
        break;
    }
}

void configure_uart(const struct device *uart_dev) {
    struct uart_config uart_cfg;

    // Pobranie bieżącej konfiguracji UART
    int ret = uart_config_get(uart_dev, &uart_cfg);
    if (ret != 0) {
        printk("Failed to get UART configuration: %d\n", ret);
        return;
    }

    // Modyfikacja konfiguracji
    uart_cfg.baudrate = 115200;           // Prędkość transmisji
    uart_cfg.parity = UART_CFG_PARITY_NONE; // Brak parzystości
    uart_cfg.stop_bits = UART_CFG_STOP_BITS_1; // Jeden bit stopu
    uart_cfg.data_bits = UART_CFG_DATA_BITS_8; // 8 bitów danych
    uart_cfg.flow_ctrl = UART_CFG_FLOW_CTRL_NONE; // Brak kontroli przepływu

    // Zastosowanie nowej konfiguracji
    ret = uart_configure(uart_dev, &uart_cfg);
    if (ret != 0) {
        printk("Failed to configure UART: %d\n", ret);
    }
}
