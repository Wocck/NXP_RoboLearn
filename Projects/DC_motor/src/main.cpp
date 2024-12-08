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
            data.joystickY = 90; // Przód
            break;
        case 1:
            data.joystickX = 20;
            data.joystickY = 60; // Skręt w prawo z ruchem do przodu
            break;
        case 2:
            data.joystickX = 90;
            data.joystickY = 0;  // Skręt w prawo w miejscu
            break;
        case 3:
            data.joystickX = 20;
            data.joystickY = -60; // Skręt w prawo z ruchem do tyłu
            break;
        case 4:
            data.joystickX = 0;
            data.joystickY = -90; // Tył
            break;
        case 5:
            data.joystickX = -20;
            data.joystickY = -60; // Skręt w lewo z ruchem do tyłu
            break;
        case 6:
            data.joystickX = -90;
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
        /* Odbieranie danych z joysticka */
        DataPacket data = getJoystickData();

        /* Sterowanie silnikami na podstawie danych z joysticka */
        engine.controlFromJoystick(data);

        /* Logowanie aktualnych wartości joysticka */
        printk("JoystickX: %d, JoystickY: %d, ButtonPressed: %d\n", 
               data.joystickX, data.joystickY, data.buttonPressed);

        /* Krótka pauza */
        k_sleep(K_MSEC(1000));
    }

    return 0;
}
