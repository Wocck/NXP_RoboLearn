#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include "uart_bridge.h"

int main(void) {
    const struct device *uart_out = DEVICE_DT_GET(DT_NODELABEL(lpuart3));
    const struct device *uart_in = DEVICE_DT_GET(DT_NODELABEL(lpuart1));

    if (!device_is_ready(uart_out)) {
        printk("UART3 device not ready\n");
        return 0;
    }

    if (!device_is_ready(uart_in)) {
        printk("UART3 device not ready\n");
        return 0;
    }

    configure_uart(uart_out);
    print_uart_config(uart_out);
    
    printk("UART Bridge started\n");

    uart_bridge_loop(uart_in, uart_out);

    return 0;
}
