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

## Przykład 1: Czujnik odległości HC-SR04

![HC-SR04 example connection](images/hcsr04_conn.png)

| Pin na płytce NXP | Pin na module HC-SR04 |
|-------------------|-----------------------|
| GND               | GND                   |
| 5V                | VCC                   |
| GPIO A4           | TRIG                  |
| GPIO A5           | ECHO                  |

**Uwaga**: Pamiętaj aby podłaczanie modułów do płytki odbywało się bez podłączenia do źródła prądu oraz zawsze najpierw podłączaj uziemienie GND.
---

### Działanie modułu HC-SR04
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

### Kwestie Hardware

Każdy hardware ma swoje mankamenty, które należy odpowiednio obsłużyć programowo. W tym przypadku jest to mierzenie odległości zerowej. Jeżeli przyłożymy przedmiot do czujnika odczytany pomiar czasu może okazać się wyjątkowo duży. Może to wynikać na przykład z tego, że czujnik nie rejestruje odbicia sygnału, mierzony czas trwa aż do osiągnięcia limitu. Z dokumentacji możemy wyczytać że czujnik jest w stanie mierzyć doległości od 2cm do 400cm co oznacza, nie wiemy jakie otrzymamy odczyty przy odległościach wykraczających poza zakres. Musimy to sprawdzić i odpowiednio oprogramować.

### Konfiguracja pinów
Aby nasz mały program był ciekawszy dodamy moduł aktywnego Buzzera aby symulować działanie czujnika parkowania w samochodzie.

| Pin na płytce NXP | Pin na module         | Pin w strukturze DTS |
|-------------------|-----------------------|----------------------|
| GND               | GND (HCSR04 i Buzzer) | &gpio1 0             |
| 5V                | VCC (HCSR04)          | &gpio1 1             |
| GPIO A3           | +    (Buzzer)         | &gpio1 21            |
| GPIO A4           | TRIG (HCSR04)         | &gpio1 17            |
| GPIO A5           | ECHO (HCSR04)         | &gpio1 16            |

Przykładowy kod implementujący działanie czujnika odległości z pikającym Buzzerem znajduje się w pliku `main_hcsr04.cpp`. 
 - Skompiluj go i uruchom na płytce.
 - Sprawdź odczyty odległości jak zmienia się ona oraz jakie się odczyty gdy dotkniemy przedmiotem do czujnika
 - Dodaj więcej trybów "pikania" aby czujnik dawał więcej informacji co do odległości
 - Zamień buzzer na diodę (pamiętaj o odpowiednim rezystorze!)

## Przykład 2: czujnik odbiciowy ST1140
Czujnik odbiciowy światła ST1140 działa, emitując wiązkę światła podczerwonego (IR) z wbudowanej diody LED. Gdy wiązka ta odbije się od powierzchni i wróci do fotodetektora (fototranzystora lub fotodioda), czujnik wykrywa obecność obiektu. Jest często używany do detekcji linii lub obiektów na krótkim dystansie, na przykład w robotyce do śledzenia linii.

| Pin na płytce NXP | Pin na module HC-SR04 |
|-------------------|-----------------------|
| GND               | GND                   |
| 5V                | VCC                   |
| GPIO A5           | S                     |

---

### Działanie modułu HC-SR04
Gdy wiązka światła odbija się od obiektu, który znajduje się w zasięgu, sygnał na wyjściu czujnika zmienia się. W zależności od koloru i rodzaju powierzchni obiektu, intensywność odbitego światła może być różna. Czarny kolor pochłania światło, więc zwraca mniejszy sygnał, podczas gdy jasne kolory odbijają go lepiej.

Aby oprogramować moduł musimy:
 - Skonfigurować pin wejściowy
 - Odczytywać w pętli stan pinu
 - Wypisać informacje na konsole aby sprawdzić działanie

 ### Ćwiczenie
 Oprogramuj moduł tak aby zwrócił informację czy napotkał kolor pochłaniający czy odbijający. Pamiętaj o użyciu poprawnych flag przy konfiguracji pinu wejściowego aby zapobiec niestabilności sygnału, gdy nie ma odbicia. Przykładowy kod znajdziesz w pliku `main_st1140.cpp`.


 ## Interfejs I2C z użyciem modułu AHT40

### Ćwiczenie

W tym ćwiczeniu zbudujemy bardziej zaawansowany program, który będzie korzystał z kilku plików źródłowych. Taki podział pozwala na lepszą organizację kodu, ułatwia jego utrzymanie oraz testowanie poszczególnych modułów. Program będzie dotyczył komunikacji I2C z użyciem modułu AHT40, który jest czujnikiem temperatury i wilgotności. W ramach ćwiczenia nauczymy się, jak skonfigurować interfejs I2C, jak komunikować się z modułem AHT40 oraz jak odczytywać i interpretować dane z tego czujnika. Przykładowy kod znajdziesz w plikach `main_i2c.cpp`, `aht40.cpp` oraz `aht40.h`.