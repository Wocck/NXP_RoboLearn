# Lab 1 - GPIO

## Ważne informacje
Przed przystąpieniem do ćwiczenia związanego z obsługą GPIO na płytce MIMXRT1064-EVK w systemie Zephyr RTOS, warto zapoznać się z podstawowymi pojęciami oraz narzędziami, które ułatwią pracę z pinami GPIO.

---

### Opis ułożenia pinów na płytce

Płytka MIMXRT1064-EVK posiada interfejs zgodny z rozkładem pinów Arduino, co ułatwia podłączanie różnych modułów i czujników. Poniżej znajduje się opis pinów dostępnych na złączach J22, J23, J24, J25:

![Arduino Interface Pinout](images/arduino_interface.png)

**Uwaga**: Nie wszystkie piny są dostępne do użytku jako GPIO, ponieważ niektóre z nich są zarezerwowane dla specjalnych funkcji (np. komunikacja I2C, SPI, UART). Jednak wiele z nich można skonfigurować jako standardowe piny GPIO.

---

### Dostęp do pinów w Zephyr RTOS

Aby używać pinów GPIO w Zephyr RTOS, musimy zrozumieć, jak system zarządza konfiguracją sprzętu za pomocą plików Device Tree (*.dts*).

**Device Tree** to struktura danych opisująca sprzęt systemu, którą system operacyjny (w tym przypadku Zephyr) wykorzystuje do konfiguracji urządzeń podczas uruchamiania. Pliki .dts (Device Tree Source) i .dtsi (Device Tree Source Include) zawierają informacje o dostępnych urządzeniach, ich adresach, przerwaniach oraz innych parametrach.

Jak działają pliki .dts i .dtsi?
- **.dtsi**: Są to pliki dołączane (include), które zawierają wspólne definicje dla różnych konfiguracji sprzętowych. Na przykład, mogą opisywać ogólną architekturę procesora.
- **.dts**: Pliki specyficzne dla danej płytki lub konfiguracji. Mogą nadpisywać lub uzupełniać informacje z plików .dtsi.

Nas interesują głównie dwa pliki .dts:

1. **Plik zephyr.dts (wygenerowany podczas kompilacji)**  
   Ten plik znajduje się w katalogu `build/zephyr/zephyr.dts` i jest rezultatem połączenia wszystkich plików `.dts` i `.dtsi` podczas procesu kompilacji. Zawiera on pełne drzewo urządzeń dla naszej aplikacji. W nim możemy znaleźć definicję kontrolera GPIO:

```dts
   gpio1: gpio@401b8000 {
       compatible = "nxp,imx-gpio";
       reg = <0x401b8000 0x4000>;
       interrupts = <0x50 0x0>, <0x51 0x0>;
       gpio-controller;
       #gpio-cells = <2>;
       pinmux = <&iomuxc_gpio_ad_b0_00_gpio1_io00>,
                <&iomuxc_gpio_ad_b0_01_gpio1_io01>,
                <&iomuxc_gpio_ad_b0_02_gpio1_io02>,
                <&iomuxc_gpio_ad_b0_03_gpio1_io03>,
                ...
       phandle = <0x102>;
   };
```

**Opis struktury `gpio1`:**

- `gpio@401b8000`: Adres bazowy rejestrów kontrolera GPIO1.
- `compatible = "nxp,imx-gpio";`: Określa zgodność z konkretnym sterownikiem GPIO dla układów NXP i.MX.
- `reg`: Zakres adresów rejestrów kontrolera GPIO.
- `interrupts`: Definiuje przerwania związane z tym kontrolerem GPIO.
- `gpio-controller`: Informuje, że ten węzeł jest kontrolerem GPIO.
- `#gpio-cells = <2>;`: Określa format używany do referencji pinów w tym kontrolerze (numer pinu i flagi).
- `pinmux`: Lista przypisań pinów (pin multiplexing), która mapuje konkretne piny do funkcji GPIO.  

2. **Plik `mimxrt1064_evk.dts` (specyficzny dla płytki)**  
    Ten plik znajduje się w katalogu `boards/arm/mimxrt1064_evk/mimxrt1064_evk.dts` i zawiera konfiguracje specyficzne dla płytki MIMXRT1064-EVK.

```dts
arduino_header: connector {
    compatible = "arduino-header-r3";
    #gpio-cells = <2>;
    gpio-map-mask = <0xffffffff 0xffffffc0>;
    gpio-map-pass-thru = <0 0x3f>;
    gpio-map = <0 0 &gpio1 26 0>,  /* A0 */
               <1 0 &gpio1 27 0>,  /* A1 */
               <2 0 &gpio1 20 0>,  /* A2 */
               <3 0 &gpio1 21 0>,  /* A3 */
               <4 0 &gpio1 17 0>,  /* A4 */
               <5 0 &gpio1 16 0>,  /* A5 */
               <6 0 &gpio1 23 0>,  /* D0 */
               <7 0 &gpio1 22 0>,  /* D1 */
               <8 0 &gpio1 11 0>,  /* D2 */
               <9 0 &gpio1 24 0>,  /* D3 */
               <10 0 &gpio1 9 0>,  /* D4 */
               <11 0 &gpio1 10 0>, /* D5 */
               <12 0 &gpio1 18 0>, /* D6 */
               <13 0 &gpio1 19 0>, /* D7 */
               <14 0 &gpio1 3 0>,  /* D8 */
               <15 0 &gpio1 2 0>,  /* D9 */
               <16 0 &gpio3 13 0>, /* D10 */
               <17 0 &gpio3 14 0>, /* D11 */
               <18 0 &gpio3 15 0>, /* D12 */
               <19 0 &gpio3 12 0>, /* D13 */
               <20 0 &gpio1 17 0>, /* D14 */
               <21 0 &gpio1 16 0>; /* D15 */
};

```

**Opis struktury `arduino_header`:**
- `connector`: Węzeł opisujący złącze, w tym przypadku kompatybilne z standardem Arduino R3.
- `compatible = "arduino-header-r3";`: Określa, że jest to złącze zgodne z Arduino R3.
- `#gpio-cells = <2>;`: Format używany do referencji pinów (numer pinu i flagi).
- `gpio-map`: Mapuje logiczne piny Arduino (A0-A5, D0-D15) na fizyczne piny kontrolerów GPIO na mikrokontrolerze.

**Przykład interpretacji `gpio-map`:**
- `<0 0 &gpio1 26 0>, /* A0 */`
  - `0 0`: Lokalny numer pinu w złączu (w tym przypadku A0).
  - `&gpio1 26 0`: Odniesienie do kontrolera `gpio1`, numer pinu 26, dodatkowe flagi 0.
  - Komentarz `/* A0 */`: Ułatwia identyfikację, że ten wpis dotyczy pinu A0.
- `<21 0 &gpio1 16 0>; /* D15 */`
  - `21 0`: Lokalny numer pinu w złączu (D15).
  - `&gpio1 16 0`: Odniesienie do kontrolera `gpio1`, numer pinu 16.
  - Komentarz `/* D15 */`: Informuje, że ten wpis dotyczy pinu D15.

---

### Plik `prj.conf` w Zephyr RTOS
Plik prj.conf jest kluczowym elementem konfiguracji projektu w Zephyr RTOS. Służy do określania ustawień kompilacji i konfiguracji systemu dla Twojej aplikacji. Dzięki niemu możesz włączyć lub wyłączyć różne funkcje i moduły Zephyr, dostosowując system do potrzeb projektu.

Pozwala na szereg funckji  w tym:
 - włączanie lub wyłączanie określonych funkcjonalności, takich jak obsługa GPIO, UART, I2C, SPI
 - zmianę rozmiaru stosu, priorytetów wątków, konfigurację pamięci
 - optymalizacja zużycia zasobów

Przykład:

```dts
CONFIG_GPIO=y
CONFIG_PRINTK=y
CONFIG_MAIN_STACK_SIZE=1024
CONFIG_CPP=y
```

Wyjaśnienie:
- `CONFIG_GPIO=y`: Włącza obsługę GPIO w systemie.
- `CONFIG_PRINTK=y`: Włącza funkcję printk() do debugowania.
- `CONFIG_MAIN_STACK_SIZE=1024`: Ustawia rozmiar stosu głównego wątku na 1024 bajty.
- `CONFIG_CPP=y` : Włącza wspieranie C++

---

### Używanie GPIO w kodzie

1. Zidentyfikuj pin na złączu Arduino: Na przykład, chcemy użyć pinu D15.
2. Sprawdź mapowanie w `arduino_header`:
```dts
<21 0 &gpio1 16 0>; /* D15 */
```
3. Użyj tych informacji w kodzie:
```C++
#define D15_NODE DT_NODELABEL(gpio1)
#define D15_PIN 

const struct device *gpio_dev = DEVICE_DT_GET(D15_NODE);

if (!device_is_ready(gpio_dev)) {
    printk("GPIO device not ready\n");
    return;
}
```

Wytłumaczenie kodu:

`struct device` jest uniwersalną strukturą reprezentującą różne urządzenia sprzętowe w systemie. Umożliwia jednolity sposób dostępu i interakcji z różnymi peryferiami, takimi jak GPIO, UART, SPI, Timer/Counters itd.

Aby pobrać wskaźnik na dane urządzenie używamt makra `DEVICE_DT_GET()` wraz z indentyfikatorem węzła w naszym przypadku `DT_NODELABEL(gpio1)`. 

Po uzyskaniu wskaźnika do struct device, należy sprawdzić czy zostało zainicjalizowane i gotowe do użycia za pomocą funckji `device_is_ready()`, która zwraca `true` jeżeli urządzenie jest gotowe do użycia.

---

### Konfiguracja pinu GPIO

#### Flagi
Kierunek pinu:
- `GPIO_INPUT` – konfiguracja pinu jako wejście.
- `GPIO_OUTPUT` – konfiguracja pinu jako wyjście.
- `GPIO_OUTPUT_LOW` – ustawienie pinu jako wyjście z domyślnym stanem niskim.
- `GPIO_OUTPUT_HIGH` – ustawienie pinu jako wyjście z domyślnym stanem wysokim.

Tryb działania:
- `GPIO_OPEN_DRAIN` – tryb otwartego drenu.
- `GPIO_PUSH_PULL` – standardowy tryb push-pull.

Rezystory podciągające:
- `GPIO_PULL_UP` – włączenie rezystora podciągającego do zasilania.
- `GPIO_PULL_DOWN` – włączenie rezystora podciągającego do masy.

Aktywność logiczna:
- `GPIO_ACTIVE_HIGH` – stan wysoki jest stanem aktywnym.
- `GPIO_ACTIVE_LOW` – stan niski jest stanem aktywnym.

Przykłady:
- Konfiguracja pinu jako wejście z rezystorem podciągającym do zasilania i aktywnością niską
  ```C++
  gpio_pin_configure(gpio_dev, PIN_NUMBER, GPIO_INPUT | GPIO_PULL_UP | GPIO_ACTIVE_LOW);
  ```
- Konfiguracja pinu jako wejście bez rezystorów podciągających
  ```C++ 
  gpio_pin_configure(gpio_dev, PIN_NUMBER, GPIO_INPUT);
  ```
- Konfiguracja pinu jako wyjście w trybie open-drain:
  ```C++
  gpio_pin_configure(gpio_dev, D15_PIN, GPIO_OUTPUT | GPIO_OPEN_DRAIN);
  ```

#### Operacje
```C++
gpio_pin_set(gpio_dev, D15_PIN, 1); // Ustawienie stanu wysokiego
gpio_pin_set(gpio_dev, D15_PIN, 0); // Ustawienie stanu niskiego
gpio_pin_toggle(gpio_dev, D15_PIN); // Przełączenie stanu pinu
```

Więcej o gpio: [GPIO Api documentation](https://docs.zephyrproject.org/apidoc/latest/group__gpio__interface.html)


## Ćwiczenie 1

## Ćwiczenie 2