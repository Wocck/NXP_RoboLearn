#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include "engine.h"

DataPacket getJoystickData() {
    // Symulacja danych z joysticka
    // W prawdziwej aplikacji, te wartości powinny pochodzić z faktycznego joysticka
    static int counter = 0;
    DataPacket data;

    // Przykładowa logika: przesuwanie joysticka w różnych kierunkach
    switch (counter % 8) {
        case 0:
            data.joystickX = 0;
            data.joystickY = 50; // Przód
            break;
        case 1:
            data.joystickX = 20;
            data.joystickY = 60; // Skręt w prawo z ruchem do przodu
            break;
        case 2:
            data.joystickX = 50;
            data.joystickY = 0;  // Skręt w prawo w miejscu
            break;
        case 3:
            data.joystickX = 20;
            data.joystickY = -60; // Skręt w prawo z ruchem do tyłu
            break;
        case 4:
            data.joystickX = 0;
            data.joystickY = -30; // Tył
            break;
        case 5:
            data.joystickX = -20;
            data.joystickY = -60; // Skręt w lewo z ruchem do tyłu
            break;
        case 6:
            data.joystickX = -30;
            data.joystickY = 0;  // Skręt w lewo w miejscu
            break;
        case 7:
            data.joystickX = -20;
            data.joystickY = 60;  // Skręt w lewo z ruchem do przodu
            break;
    }

    data.buttonPressed = 0; // Brak naciskania przycisku
    counter++;
    return data;
}

uint32_t mapSpeedToPulse(uint8_t speed) {
    // Zakres wejściowy: 0 - 90
    if(speed == 0) {
        return 0;
    }
    const uint32_t min_pulse_ns = 35000; // Minimalne pulse_ns dla uruchomienia silnika
    const uint32_t max_pulse_ns = 50000; // Maksymalne pulse_ns
    const uint8_t max_speed = 90;        // Maksymalna wartość wejściowa

    // Zabezpieczenie przed przekroczeniem zakresu
    if (speed > max_speed) {
        speed = max_speed;
    }

    // Mapowanie liniowe: pulse_ns = min_pulse_ns + (speed / max_speed) * (max_pulse_ns - min_pulse_ns)
    uint32_t pulse_ns = min_pulse_ns + ((max_pulse_ns - min_pulse_ns) * speed) / max_speed;

    return pulse_ns;
}

int main(void)
{
    /* Utworzenie instancji klasy Engine */
    Engine engine;

    /* Inicjalizacja PWM i GPIO */
    if (!engine.init()) {
        printk("Engine initialization failed\n");
        return -1;
    }

    printk("Starting PWM and Motor Control...\n");
    while (1) {
        // Pobierz dane joysticka
        DataPacket joystickData = getJoystickData();

        // Steruj silnikami
        engine.controlMotors(joystickData);
        printk("Joystick X: %d, Y: %d\n", joystickData.joystickX, joystickData.joystickY);

        k_sleep(K_MSEC(3000)); // Krótka pauza
    }

    return 0;
}
