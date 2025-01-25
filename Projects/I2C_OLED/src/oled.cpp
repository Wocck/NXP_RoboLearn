#include "oled.h"
#include <zephyr/kernel.h>  // k_sleep, K_MSEC

Oled::Oled(const struct device *i2c_dev)
    : m_i2c_dev(i2c_dev)
{
}

void Oled::sendCommand(uint8_t cmd) {
    uint8_t buf[2];
    buf[0] = 0x00;
    buf[1] = cmd;

    i2c_write(m_i2c_dev, buf, 2, SH1106_I2C_ADDR);
}

void Oled::sendCommand(const uint8_t *cmds, size_t len) {
    for (size_t i = 0; i < len; i++) {
        sendCommand(cmds[i]);
    }
}

void Oled::sendData(uint8_t data) {
    uint8_t buf[2];
    buf[0] = 0x40;
    buf[1] = data;

    i2c_write(m_i2c_dev, buf, 2, SH1106_I2C_ADDR);
}

void Oled::setPageColumn(uint8_t page, uint8_t col) {
    sendCommand(0xB0 + page);

    uint8_t low_col = 0x00 + (col & 0x0F);
    uint8_t high_col = 0x10 + ((col >> 4) & 0x0F);

    sendCommand(low_col);
    sendCommand(high_col);
}

void Oled::init() {
    static const uint8_t init_cmds[] = {
        0xAE, // Display OFF
        0xD5, 0x80, // Set display clock divide ratio/oscillator freq
        0xA8, 0x3F, // Multiplex ratio: 0x3F = 64 MUX
        0xD3, 0x00, // Display offset: 0
        0x40,       // Display start line: 0
        0xAD, 0x8B, // DC-DC control mode set, DC-DC on
        // Remap, COM scan direction
        0xA1,       // Segment remap: 0xA0=normal, 0xA1=remap
        0xC8,       // COM Output scan direction: 0xC0=normal, 0xC8=inverted
        0xDA, 0x12, // COM pins hardware config
        0x81, 0x7F, // Contrast control
        0xD9, 0x22, // Pre-charge period
        0xDB, 0x35, // VCOMH deselect level
        0xA6,       // Normal display (not inverted)
        0xAF        // Display ON
    };
    sendCommand(init_cmds, sizeof(init_cmds));

    k_sleep(K_MSEC(100));
}


void Oled::fill(uint8_t data) {
    for (uint8_t page = 0; page < 8; page++) {
        setPageColumn(page, 0);
        for (uint8_t col = 0; col < 128; col++) {
            sendData(data);
        }
    }
}

void Oled::entireOn() {
    sendCommand(0xA5);
}

void Oled::normalDisplay() {
    sendCommand(0xA4);
}
