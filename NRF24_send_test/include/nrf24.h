#ifndef NRF24_H
#define NRF24_H

#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>

struct DataPacket {
    int8_t joystickX;
    int8_t joystickY;
    uint8_t buttonPressed : 1;
};

// Funkcje do obsługi nRF24L01
int nrf24l01_init(const struct device *spi_dev);
int nrf24l01_receive_payload(struct DataPacket *packet);
int nrf24l01_send_payload(const uint8_t *data, size_t len);
void nrf24l01_test_registers(void);

#endif // NRF24_H
