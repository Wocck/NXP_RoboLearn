# Lab 2 - Serial

## Wstęp

1. MIMXRT1064_EVK Pinout:

![Arduino Interface Pinout](images/arduino_interface.png)

2. Wykorzystywane moduły i ich dokumentacje
 - Ultradżwiękowy czujnik odległości **HC-SR04** : [Datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)
 - Czujnik dbiciowy optyczny **ST1140**: [Datasheet](https://cdn-reichelt.de/documents/datenblatt/C200/ST1140.pdf)
 - Wyświetlacz LCD 1.8'' **ST7735S**: [Datasheet]()
 - Czujnik temperatury i wilgotności **AHT20**: [Datasheet]()
 - Moduł radiowy **nRF24L01**: [Datasheet](http://www.nordicsemi.com/eng/Products/2.4GHz-RF/nRF24L01)

## Urządzenia nie wymagające protokołu komunikacyjnego
Zaczynamy od urządzeń nie wymagających konkretnego protokołu komunikacyjnego, ponieważ są one prostsze w implementacji i pozwalają na szybkie zapoznanie się z działaniem komunikacji szeregowej. Urządzenia te posiadają własne uproszczone protokoły, które zwykle sprowadzają się do ustawienia jakiejś wartości na pinie i oczekiwaniu aż moduł zwróci jakieś dane. Oznacza to że przujmują one jedynie proste komendy wymagające odczytu stanu na danym pinie.

Te moduły to: **HC-SR04** oraz **ST1140**.

## Przykładowe połączenie modułu

![HC-SR04 example connection](images/hcsr04_conn.png)

| Pin na płytce NXP | Pin na module HC-SR04 |
|-------------------|-----------------------|
| GND               | GND                   |
| 5V                | VCC                   |
| GPIO A4           | TRIG                  |
| GPIO A5           | ECHO                  |

**Uwaga**: Pamiętaj aby podłaczanie modułów do płytki odbywało się bez podłączenia do źródła prądu oraz zawsze najpierw podłączaj uziemienie GND.
---

## Działanie modułu HC-SR04
Moduł HC-SR04 to ultradźwiękowy czujnik odległości, który mierzy czas przelotu fal dźwiękowych między czujnikiem a przeszkodą, co pozwala na obliczenie odległości. Działa w dwóch etapach:
1. **Wysyłanie impulsu** – Czujnik generuje krótki impuls ultradźwiękowy o częstotliwości 40 kHz przez pin *TRIG*, który trwa 10 mikrosekund.

2. **Odbiór echa** – Jeśli fala dźwiękowa odbije się od przeszkody, czujnik rejestruje czas powrotu sygnału na pinie ECHO. Na podstawie tego czasu można obliczyć odległość od przeszkody, używając wzoru:
$$
\text{odległość [m]} = \frac{\text{czas [s]} \times \text{prędkość dźwięku w powietrzu} \left[ \frac{m}{s} \right]}{2}
$$

Oznacza to że programistycznie aby mierzyć odległość musimy:
1. Ustawić pin *TRIGG* na stan wysoki na 10 mikrosekund.
2. Po wysłaniu fali czujnik odrazu ustawi pin ECHO w stan wysoki.
3. Gdy fala wróci czujnik zmieni stan pinu ECHO na niski. 
4. Nasz program musi zmierzyć czas trwania sygnału wysokiego na pinie ECHO i obliczyć odległość z powyższego wzoru.

## Konfiguracja pinów
Aby nasz mały program był ciekawszy dodamy moduł aktywnego Buzzera aby symulować działanie czujnika parkowania w samochodzie.

| Pin na płytce NXP | Pin na module         | Pin w strukturze DTS |
|-------------------|-----------------------|----------------------|
| GND               | GND (HCSR04 i Buzzer) | &gpio1 0             |
| 5V                | VCC (HCSR04)          | &gpio1 1             |
| GPIO A3           | +    (Buzzer)         | &gpio1 21            |
| GPIO A4           | TRIG (HCSR04)         | &gpio1 17            |
| GPIO A5           | ECHO (HCSR04)         | &gpio1 16            |

## Konfiguracja prj.conf