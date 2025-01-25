#ifndef OLED_H_
#define OLED_H_

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>

#define SH1106_I2C_ADDR 0x3C

class Oled {
public:

    Oled(const struct device *i2c_dev);

    void init();

    // Wypełnia pamięć ekranu podanym bajtem (0x00 = czarny, 0xFF = biały)
    void fill(uint8_t data);

    // Wyświetlacz w trybie "entire display ON" (wszystkie piksele zapalone)
    void entireOn();

    // Wyświetlacz w trybie "normal display" (pokazuje zawartość pamięci GDDRAM)
    void normalDisplay();

private:
    const struct device *m_i2c_dev;

    void sendCommand(uint8_t cmd);

    void sendCommand(const uint8_t *cmds, size_t len);

    void sendData(uint8_t data);

    void setPageColumn(uint8_t page, uint8_t col);
};

#endif /* OLED_H_ */
