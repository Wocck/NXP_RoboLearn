#ifndef NRF24_H
#define NRF24_H

#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>

struct DataPacket {
    int16_t joystickX;
    int16_t joystickY;
    uint8_t buttonPressed;
}__attribute__((packed));

// Funkcje do obsługi nRF24L01
int nrf24l01_init(const struct device *spi_dev);
int nrf24l01_receive_payload(struct DataPacket *packet);
void nrf24l01_test_registers(void);

#endif // NRF24_H
