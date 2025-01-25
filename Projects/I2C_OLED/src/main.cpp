#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>

#include "oled.h"

int main(void)
{
    printk("Start programu: SH1106 test via I2C\n");

    const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(lpi2c1));
    if (!device_is_ready(i2c_dev)) {
        printk("Błąd: urządzenie I2C (lpi2c1) nie jest gotowe!\n");
        return -1;
    }

    Oled oled(i2c_dev);
    oled.init();

    oled.fill(0x00);

    while (true) {
        oled.entireOn();
        k_sleep(K_MSEC(2000));

        oled.normalDisplay();
        k_sleep(K_MSEC(2000));
    }
    return 0;
}
