#ifndef DATAPACKET_H
#define DATAPACKET_H

#include <stdint.h>

/**
 * @struct DataPacket
 * @brief Data structure representing a packet sent/received via the nRF24L01+ module.
 */
struct DataPacket {
    int8_t joystickX; ///< X-axis value of the joystick
    int8_t joystickY; ///< Y-axis value of the joystick
    uint8_t buttonPressed; ///< Button pressed status (0 - not pressed, 1 - pressed)
} __attribute__((packed));

#endif // DATAPACKET_H