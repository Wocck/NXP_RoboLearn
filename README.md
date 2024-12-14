# Lab 2 - Serial

## Wstęp

1. MIMXRT1064_EVK Pinout:

![Arduino Interface Pinout](docs/images/arduino_interface.png)

2. Wykorzystywane moduły i ich dokumentacje
 - Ultradżwiękowy czujnik odległości **HC-SR04** : [Datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)
 - Czujnik dbiciowy optyczny **ST1140**: [Datasheet](https://cdn-reichelt.de/documents/datenblatt/C200/ST1140.pdf)
 - Wyświetlacz LCD 1.8'' **ST7735S**: [Datasheet]()
 - Czujnik temperatury i wilgotności **AHT20**: [Datasheet]()
 - Moduł radiowy **nRF24L01**: [Datasheet](http://www.nordicsemi.com/eng/Products/2.4GHz-RF/nRF24L01)

## Device Tree w Zephyr

**DTS (Device Tree Source)** - to struktura opisująca sprzętową konfigurację mikrokontrolera w systemie Zephyr. DTS jest plikiem tekstowym, który zawiera informacje o sprzęcie, takie jak piny GPIO, kontrolery, interfejsy SPI czy UART, itp. W uproszczeniu pliki źródłowe DTS są używane do wygenerowania pliku nagłówkowego w formacie `.h`. Zawartość wygenerowanego nagłówka jest abstrahowana przez API `devicetree.h`, którego można użyć do uzyskania informacji z devicetree.

DTS w Zephyr składa się z następujących elementów:

- Sources (`.dts`) - to główne pliki opisujące konfigurację sprzętową dla danej płytki. Określają, które urządzenia są włączone (status = "okay") i przypisują odpowiednie piny. Na przykład aktywuje pierwszy interfejs I2C i przypisuje mu konfigurację pinów zdefiniowaną w pliku `.dtsi`. `aht20@38` Deklaruje czujnik AHT20 podłączony do lpi2c1 z adresem 0x38
```dts
&lpi2c1 {
    status = "okay";
    pinctrl-0 = <&pinmux_lpi2c1>;
    pinctrl-names = "default";

    aht20: aht20@38 {
        compatible = "aosong,aht20";
        reg = <0x38>;
    };
};
```

- Includes (`.dtsi`) - zawierają ogólne/wspólne dane, które mogą być współdzielone przez wiele plików dts. Każdy plik dts zawiera jeden lub wiele plików dtsi i wprowadza zmiany specyficzne dla tego pliku dts. Na przykład definiuje szczegóły konfiguracji pinów dla `lpi2c1`, takie jak funkcje pinów, tryb pracy czy siła sygnału.
```dts
  pinctrl_lpi2c1: pinmux_lpi2c1 {
    group0 {
        pinmux = <&iomuxc_gpio_ad_b1_01_lpi2c1_sda>,
                 <&iomuxc_gpio_ad_b1_00_lpi2c1_scl>;
        drive-strength = "r0-6";
        drive-open-drain;
        nxp,speed = "100-mhz";
    };
};
```
- Overlays (`.overlay`) -  pozwalają modyfikować lub rozszerzać konfigurację sprzętową zdefiniowaną w plikach `.dts` i `.dtsi`. Są używane do wprowadzania zmian specyficznych dla aplikacji, bez potrzeby edytowania oryginalnych plików. Na przykład modyfikuje istniejący węzeł lpi2c1 dodając do niego konfigurację dla czujnika AHT20.
```dts
&lpi2c1 {
    status = "okay";
    aht20: aht20@38 {
        compatible = "aosong,aht20";
        reg = <0x38>;
    };
};
```  

- Bindings (`.yaml`) - zawierają tzw. "bindings", czyli metadane opisujące, jak DTS ma być interpretowane przez Zephyra. Określają, jakie właściwości (properties) są wymagane, jakie są ich domyślne wartości, i jak mają być mapowane na funkcje w systemie. Na przykład informuje Zephyra, że urządzenie z compatible = "aosong,aht20" to czujnik AHT20 i wymaga właściwości reg (adresu I2C).
```yaml
compatible: "aosong,aht20"
description: AHT20 temperature and humidity sensor
properties:
  reg:
    type: int
    required: true
    description: I2C address of the device
  reset-gpios:
    type: phandle-array
    required: false
    description: GPIO pin to reset the sensor
```

## Plik `mimxrt1064_evk.dts`

1. **Aliasy** - umożliwiają tworzenie alternatywnych nazw dla węzłów (ang. nodes), co upraszcza odwoływanie się do nich w kodzie.
```dts
	aliases {
		led0 = &green_led;
		pwm-led0 = &green_pwm_led;
        ...
	};
```
Dzięki temu w kodzie możesz używać `DT_ALIAS(led0)` zamiast `DT_NODELABEL(green_led)`, co jest bardziej czytelne.

2. **chosen** - wskazuje domyślne urządzenia lub konfiguracje, które mają być używane przez system.
```dts
chosen {
    zephyr,uart-mcumgr = &lpuart1;
    zephyr,console = &lpuart1;
    zephyr,shell-uart = &lpuart1;
    zephyr,canbus = &flexcan2;
};
```
Na przykład `zephyr,console = &lpuart1;` ustawia `lpuart1` jako domyślne urządzenie  konsoli. Dzięki temu używając `printf()` czy `cout` w kodzie dane będą wypisywane na `lpuart1`.

3. **nodes** - definiują węzły, które reprezentują urządzenia sprzętowe. Każdy węzeł ma swoje właściwości (properties), które opisują konfigurację sprzętową urządzenia.
```dts
nxp_i2c_touch_fpc: i2c-touch-connector {
    compatible = "nxp,i2c-tsc-fpc";
    #gpio-cells = <2>;
    gpio-map-mask = <0xffffffff 0xffffffc0>;
    gpio-map-pass-thru = <0 0x3f>;
    gpio-map =	<1  0 &gpio1 2 0>,	/* Pin 2, LCD touch RST */
            <2  0 &gpio1 11 0>;	/* Pin 3, LCD touch INT */
};
```
Węzeł `nxp_i2c_touch_fpc` reprezentuje interfejs I2C do ekranu dotykowego. Właściwość `compatible` określa, że jest to interfejs do ekranu dotykowego firmy NXP. Właściwość `#gpio-cells` określa, że węzeł ma dwa elementy typu `gpio`. Właściwość `gpio-map` przypisuje piny GPIO do konkretnych funkcji.

```dts
leds {
    compatible = "gpio-leds";
    green_led: led-1 {
        gpios = <&gpio1 9 GPIO_ACTIVE_LOW>;
        label = "User LD1";
    };
};
```
Węzeł leds opisuje diody LED obsługiwane przez GPIO na płytce. W tym przypadku opisuje pojedynczą diodę LED oznaczoną jako green_led.
- `compatible = "gpio-leds";` - Informuje, że ten węzeł jest kompatybilny z generatorem sterowników dla diod LED sterowanych przez GPIO.
- `green_led: led-1 {}` - Definiuje konkretną diodę LED o nazwie `green_led` i przypisuje jej alias `led-1`.

## Urządzenia nie wymagające protokołu komunikacyjnego
Zaczynamy od urządzeń nie wymagających konkretnego protokołu komunikacyjnego, ponieważ są one prostsze w implementacji i pozwalają na szybkie zapoznanie się z działaniem komunikacji szeregowej. Urządzenia te posiadają własne uproszczone protokoły, które zwykle sprowadzają się do ustawienia jakiejś wartości na pinie i oczekiwaniu aż moduł zwróci jakieś dane. Oznacza to że przujmują one jedynie proste komendy wymagające odczytu stanu na danym pinie.

Te moduły to: **HC-SR04** oraz **ST1140**.

## Przykład 1: Czujnik odległości HC-SR04

![HC-SR04 example connection](docs/images/hcsr04_conn.png)

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

### Działanie modułu ST1140
Gdy wiązka światła odbija się od obiektu, który znajduje się w zasięgu, sygnał na wyjściu czujnika zmienia się. W zależności od koloru i rodzaju powierzchni obiektu, intensywność odbitego światła może być różna. Czarny kolor pochłania światło, więc zwraca mniejszy sygnał, podczas gdy jasne kolory odbijają go lepiej.

Aby oprogramować moduł musimy:
 - Skonfigurować pin wejściowy
 - Odczytywać w pętli stan pinu
 - Wypisać informacje na konsole aby sprawdzić działanie

 ### Ćwiczenie
 Oprogramuj moduł tak aby zwrócił informację czy napotkał kolor pochłaniający czy odbijający. Pamiętaj o użyciu poprawnych flag przy konfiguracji pinu wejściowego aby zapobiec niestabilności sygnału, gdy nie ma odbicia. Przykładowy kod znajdziesz w pliku `main_st1140.cpp`.


## Interfejs I2C z użyciem modułu AHT40

W tym ćwiczeniu zbudujemy bardziej zaawansowany program, który będzie korzystał z kilku plików źródłowych. Taki podział pozwala na lepszą organizację kodu, ułatwia jego utrzymanie oraz testowanie poszczególnych modułów. Program będzie dotyczył komunikacji I2C z użyciem modułu AHT40, który jest czujnikiem temperatury i wilgotności. W ramach ćwiczenia nauczymy się, jak skonfigurować interfejs I2C, jak komunikować się z modułem AHT40 oraz jak odczytywać i interpretować dane z tego czujnika. Przykładowy kod znajdziesz w plikach `main_i2c.cpp`, `aht40.cpp`, `aht40.h` oraz `mimxrt1064_evk.overlay`.

#### Konfiguracja interfejsu I2C
W pliku `mimxrt1064_evk.overlay` dodajemy konfigurację interfejsu I2C:
```dts
&lpi2c1 {
    status = "okay";
    pinctrl-0 = <&pinmux_lpi2c1>;
    pinctrl-names = "default";
};
```

** Wytłumaczenie**
- `&lpi2c1` - Węzeł reprezentujący interfejs I2C1, należy zmienić jego pole `status` na "okay", aby włączyć interfejs.
- `pinctrl-0` - Węzeł definiujący konfigurację pinów dla interfejsu I2C1. `pinmux_lpi2c1` to alias do węzła zdefiniowanego w `mimxrt1064_evk-pinctrl.dtsi`.
- `pinctrl-names` - Wskazuje że będziemy używać pinów w konfiguracji domyślnej.


**Uwaga** - adres `0x70` to adres urządzenia w formacie 8-bitowym, takiego wymaga dokumentacja Zephyr dla urządzeń I2C, który uwzględnia bit R/W w najmłodszej pozycji. W przypadku AHT20, adres ten jest zapisywany jako `0x38` w formacie 7-bitowym. W kodzie będziemy używać adresu `0x38`.

#### I2C w Zephyr

Na początku musimy pobrać konfigurację kontrolera I2C zdefiniowanego w pliku DTS: `const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(lpi2c1));`
Poniżej znajdują się przydane funckje do obsługi I2C:
- `int i2c_write(const struct device *dev, const uint8_t *buf, uint32_t num_bytes, uint16_t addr);` - zapisuje dane do urządzenia pod 7-bitowym adresem `addr`.
- `int i2c_read(const struct device *dev, uint8_t *buf, uint32_t num_bytes, uint16_t addr);` - odczytuje dane z urządzenia pod 7-bitowym adresem `addr`.
- `int i2c_burst_read(const struct device *dev, uint16_t addr, uint8_t start_addr, uint8_t *buf, uint32_t num_bytes);` - odczytuje dane z urządzenia pod 7-bitowym adresem `addr` zaczynając od adresu `start_addr`.
- `int i2c_burst_write(const struct device *dev, uint16_t addr, uint8_t start_addr, uint8_t *buf, uint32_t num_bytes);` - zapisuje dane do urządzenia pod 7-bitowym adresem `addr` zaczynając od adresu `start_addr`.
- `int i2c_reg_write_byte(const struct device *dev, uint16_t addr, uint8_t reg, uint8_t value);` - Wysyła jeden bajt danych do wskazanego rejestru urządzenia I2C.
- `int i2c_reg_read_byte(const struct device *dev, uint16_t addr, uint8_t reg, uint8_t *value);`- Odczytuje jeden bajt danych ze wskazanego rejestru urządzenia I2C.

Dokumentację I2C w Zephyr znajdziesz [tutaj](https://docs.zephyrproject.org/apidoc/latest/group__i2c__interface.html).
Lub w pliku `zephyrproject\zephyr\include\zephyr\drivers\i2c.h`

#### Oprogramowanie modułu - ćwiczenie

1. W dokumentacji na stronie 8 możemy wyczytać instrukcje odczytu danych z czujnika. Znajdź i przeczytaj je.
2. Zaimplementuj funkcję która odczyta statusu kalibracji przy uruchomieniu zgodnie z dokumentacją:
   - rejestr `0x71`.
   - metoda odczytu rejestru `i2c_reg_read_byte()`.
3. Zaimplementuj funkcję inicjalizacji czujnika zgodnie z dokumentacją:
   - Jeśli bit [3] statusu kalibracji odczytanego z rejestru `0x71` jest równy `0`, należy wysłać polecenie inicjalizacyjne `0xBE`.
   - Komenda `0xBE` wymaga przesłania dwóch parametrów: pierwszy bajt `0x08`, drugi bajt `0x00`.
   - Po wysłaniu komendy poczekaj 10 ms na zakończenie inicjalizacji.

4. Zaimplementuj funkcję wyzwalania pomiaru i sprawdzania jego zakończenia:
   - Wyślij komendę `0xAC` wraz z parametrami: pierwszy bajt `0x33`, drugi bajt `0x00`.
   - Po wysłaniu komendy poczekaj, aż pomiar się zakończy:
     - Sprawdzaj bit [7] w rejestrze statusu `0x71`. Jeśli jest równy `0`, pomiar został zakończony.
     - Jeśli bit [7] jest równy `1`, czekaj i sprawdzaj ponownie (np. w odstępach co 10 ms).

5. Zaimplementuj funkcję odczytu wyników pomiaru:
   - Po zakończeniu pomiaru odczytaj 6 bajtów danych z czujnika. Dane te zawierają:
     - Wilgotność względna: 20-bitowa wartość w bajtach [1], [2], [3] (najstarsze bity w [1], najmłodsze w [3]).
     - Temperatura: 20-bitowa wartość w bajtach [3], [4], [5] (najstarsze bity w [3], najmłodsze w [5]).
   - Oblicz wartości wilgotności i temperatury:
     - Wilgotność względna = `(wartość wilgotności / 1048576) * 100 [%]`.
     - Temperatura = `(wartość temperatury / 1048576) * 200 - 50 [°C]`.

## Interfejs SPI z użyciem modułu nRF24L01

### Schemat Połączeń

| **Pin na płytce NXP** | **Pin GPIO (RT1064)** | **Mapowanie pinu z DTS**   | **Pin na module nRF24L01** | **Opis**               |
|-----------------------|-----------------------|----------------------------|----------------------------|------------------------|
| GND                   | GND                   |                            | GND                        | Masa                   |
| 3.3V                  | 3V                    |                            | VCC                        | Zasilanie (+3.3V)      |
| GPIO_AD_B0_03         | D9                    | &gpio1   2                 | CE                         | Włącz odbiornik        |
| GPIO_SD_B0_01         | D10                   | &gpio3   13                | CSN                        | Chip Select            |
| GPIO_SD_B0_02         | D11                   | &gpio3   14                | MOSI                       | Dane wysyłane do modułu|
| GPIO_SD_B0_03         | D12                   | &gpio3   15                | MISO                       | Dane odbierane z modułu|
| GPIO_SD_B0_00         | D13                   | &gpio3   12                | SCK                        | Zegar SPI              |

### Konfiguracja SPI
W pliku `mimxrt1064_evk.overlay` dodajemy konfigurację dla SPI:

```dts
&lpspi1 {
    status = "okay";
    cs-gpios = <&gpio3 13 GPIO_ACTIVE_LOW>;
};

&gpio1 {
    status = "okay";
};

&gpio3 {
    status = "okay";
};
```

**Wytłumaczenie:**
- `&lpspi1`: Węzeł reprezentujący kontroler SPI1. Zmieniamy `status` na `"okay"`, aby włączyć SPI.
- `cs-gpios`: Definiuje pin GPIO, który pełni funkcję Chip Select (CSN) dla modułu nRF24L01.
- `&gpio1` i `&gpio3`: Aktywują magistrale GPIO potrzebne do obsługi pinów CE i CSN.

### Podstawy komunikacji SPI

SPI (Serial Peripheral Interface) to protokół komunikacyjny typu master-slave, w którym master (mikrokontroler) steruje przesyłaniem danych. Interfejs SPI wymaga kilku linii:
1. **SCK (zegar):** Synchronizuje transfer danych.
2. **MOSI (Master Out Slave In):** Dane przesyłane od master do slave.
3. **MISO (Master In Slave Out):** Dane przesyłane od slave do master.
4. **CSN (Chip Select):** Wybiera aktywne urządzenie slave.

### Przydatne funkcje Zephyr SPI API
- `spi_transceive`: Realizuje jednoczesne wysyłanie i odbieranie danych w ramach jednego transferu.
- `spi_write`: Przesyła dane bez odbioru.
- `spi_read`: Odbiera dane bez przesyłania.
- `gpio_pin_set`: Służy do sterowania pinem CE w nRF24L01.

Dokumentacja SPI w Zephyr znajduje się [tutaj](https://docs.zephyrproject.org/apidoc/latest/group__spi__interface.html).
Lub w pliku `zephyrproject\zephyr\include\zephyr\drivers\spi.h`

### Cwiczenie - Odbiór danych w trybie pooling z nRF24L01

1. Skonfigurować interfejs SPI do komunikacji z modułem:
    - SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE | SPI_HOLD_ON_CS;
    - Ustawienie częstotliwości zegara na 100kHz.
    - Usatwienie pinu CS
np.
```C++
spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE | SPI_HOLD_ON_CS;
spi_cfg.frequency = 100000;
spi_cfg.slave = 0U;
spi_cfg.cs.gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(lpspi1), cs_gpios);
```

1. Ustawić domyślne wartości rejestrów modułu, takie jak:
    - `CONFIG_REG`: Włącz tryb odbiornika i odpowiednie przerwania.
    - `EN_RXADDR`: Aktywuj odpowiednie kanały RX.
    - `SETUP_AW`: Określ szerokość adresu.
    - `RF_CH`: Ustaw częstotliwość.
    - `RF_SETUP`: Skonfiguruj moc i prędkość transmisji.
Wszystkie wartości rejestrów możemy odczytać z tabeli `Register map table` na stronie 54 w dokumentacji modułu.

**Uwaga**
1. Podczas konfiguracji rejestró należy ustawić pin CE na stan niski a po konfiguracji w celu uruchomienia trybu odbioru należy ustawić go na stan Wysoki.
2. Moduł nRF24L01 wymaga zasilania 3.3V, nie podłączaj go do zasilania 5V.
3. W przypadku problemów z komunikacją, sprawdź połączenia, konfigurację pinów i rejestrów oraz czy moduł jest zasilany. W przypadku błędnej konfiguracji modułu w kodzie czasem pomaga odłączenie i podłączenie zasilania.
4. Wartosci rejestrów należy ustawiać za pomocą wysania wartości `uint8_t cmd = 0x20 | (reg & 0x1F);` przez SPI. Gdzie `reg` to adres rejestru, a `cmd` to komenda zapisu do rejestru. 

### Ważne rejestry nRF24L01
1. **CONFIG_REG (0x00):**
   - Bit [0]: Tryb pracy (1 = RX, 0 = TX).
   - Bit [1]: Włącz przerwania RX.

2. **STATUS_REG (0x07):**
   - Bit [6]: Flaga RX_DR (dane gotowe do odbioru).
   - Bit [4]: Flaga TX_DS (wysłano poprawnie).

3. **RX_ADDR_P0 (0x0A):**
   - Adres odbiorcy dla kanału RX0 (do 5 bajtów).

4. **RF_CH (0x05):**
   - Częstotliwość pracy modułu (2400 MHz + `RF_CH`).

5. **RX_PW_P0 (0x11):**
   - Liczba bajtów danych odbieranych w kanału RX0.

---

### Ćwiczenie

1. Połącz mikrokontroler z modułem nRF24L01 zgodnie ze schematem połączeń.
2. W dokumentacji nRF24L01 znajdź opisy rejestrów `CONFIG_REG`, `STATUS_REG` i `RX_ADDR_P0`. Przeczytaj, jak wpływają na działanie modułu.
3. Zaimplementuj funkcję inicjalizacji modułu nRF24L01:
   - Skonfiguruj domyślne wartości rejestrów.
   - Zresetuj flagi w rejestrze STATUS.
   - Włącz odbiornik, ustawiając CE na wysoki.
4. Zaimplementuj funkcję odczytu danych:
   - Sprawdź flagę RX_DR w rejestrze STATUS.
   - Jeśli dane są gotowe, odczytaj payload z `R_RX_PAYLOAD`.
   - Wyświetl odebrane dane w konsoli.
5. Przetestuj program, przesyłając dane z nadajnika (np. joysticka) i odbierając je na odbiorniku.