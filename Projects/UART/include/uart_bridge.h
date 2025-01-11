#ifndef UART_BRIDGE_H
#define UART_BRIDGE_H

#include <zephyr/device.h>

void uart_bridge_loop(const struct device *uart_in, const struct device *uart_out);
void configure_uart(const struct device *uart_dev);
void print_uart_config(const struct device *uart_dev);

#endif // UART_BRIDGE_H