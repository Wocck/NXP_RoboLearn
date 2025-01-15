# Moduł opcjonalny - Lidar

W ramach tego ćwiczenia zrealizujemy uproszczony system LIDAR, wykorzystując serwomechanizm oraz ultradźwiękowy czujnik odległości HC-SR04. Celem ćwiczenia jest nauka integracji czujnika z ruchomą platformą, co pozwoli na skanowanie otoczenia i tworzenie map odległości.

## Przydatne pliki
- [Reference Manual](docs/MIMXRT1064_Reference_Manual.pdf)
- [mimxrt1064_evk.dts](..\zephyrproject\zephyr\boards\nxp\mimxrt1064_evk\mimxrt1064_evk.dts)
- [mimxrt1064_evk-pinctrl.dtsi](..\zephyrproject\zephyr\boards\nxp\mimxrt1064_evk\mimxrt1064_evk-pinctrl.dtsi)
- [fsl_iomuxc.h](..\zephyrproject\modules\hal\nxp\mcux\mcux-sdk\devices\MIMXRT1064\drivers\fsl_iomuxc.h)

## Cele

- Poznanie zasad działania serwomechanizmów
- Wykorzystanie PWM do sterowania kątem obrotu serwomechanizmu
- Implementacja algorytmu skanowania i rejestracji danych o odległości
- Implementacja wysyłania odpowiedzi do kontrolera w postaci mapy odległości

## Wymagane komponenty

- Platforma robota z modułem radiowym NRF24L01 oraz mikrokontrolerem MIMXRT1064-EVK
- Serwomechanizm SG90
- Czujnik odległości HC-SR04
- Kontroler ESP32 z ekranem OLED, Joystickiem oraz modułem radiowym NRF24L01

## Lidar - zasada działania

Lidar (Light Detection and Ranging) to urządzenie służące do pomiaru odległości za pomocą promieniowania elektromagnetycznego. W naszym przypadku zamiast lasera wykorzystamy ultradźwiękowy czujnik odległości HC-SR04. W celu skanowania otoczenia, będziemy obracać serwomechanizmem, na którym zamocujemy czujnik. W ten sposób uzyskamy możliwość pomiaru odległości w różnych kierunkach. Porządanym efektem jest uzyskanie mapy odległości, którą będziemy przesyłać do kontrolera ESP32 a on będzie wyświetlał zwrotne dane na ekranie OLED. Może to wyglądać na przykłąd w ten sposób:

![Lidar - przykład](images/lidar.jpg)

## Serwomechanizm SG90

Serwomechanizm SG90 to urządzenie elektromechaniczne służące do precyzyjnego sterowania kątem obrotu. Składa się z:

1. **Silnika DC** – generuje ruch obrotowy.
2. **Przekładni zębatej** – zmniejsza prędkość obrotową i zwiększa moment obrotowy.
3. **Układu sterującego** – odbiera sygnał sterujący PWM i ustawia kąt.
4. **Potencjometru** – pełni rolę czujnika pozycji, umożliwiając kontrolowanie aktualnego kąta.

Serwomechanizm interpretuje sygnały PWM (Pulse Width Modulation) do określania kąta obrotu wału. Długość impulsu w sygnale PWM definiuje docelowy kąt. Typowy zakres kątów wynosi od 0° do 180°.

Sygnał PWM składa się z cyklu powtarzających się impulsów. Każdy cykl trwa typowo **20 ms** (50 Hz). Długość impulsu wysokiego w ramach cyklu definiuje kąt obrotu:

- **0.5 ms** – wał ustawiony na 0°.
- **1.5 ms** – wał ustawiony na 90°.
- **2.5 ms** – wał ustawiony na 180°.

Długość impulsu PWM $T_\text{impuls}$ (w ms) dla danego kąta $\theta$ (w stopniach) można obliczyć za pomocą wzoru liniowego:

$$
T_\text{impuls} = 0.5 + \frac{\theta}{180} \times 2.0
$$

## Etap 1: Konfiguracja środowiska

W projekcie będziemy korzystać z już zaimplementowanych funkcjonalności w naszym robocie czyli z klasy `nrf24.h`, `engine.h` oraz `hcsr04.h`. Niektóre z nich będzie należało lekko zmodyfikować, aby pasowały do naszych potrzeb. Mamy już skonfigurowane 2 piny PWM do sterowania silnikami DC za pomocą klastry `engine`. 
Teraz musimy dodać kolejny pin PWM do sterowania serwomechanizmem. 

1. Identyfikacja numeru pinu:
    - Wybieramy wolny pin zdolny do generowania sygnału PWM (patrz `images/arduino_interface.png`), np. `D5`.
    - Sprawdzamy faktyczny numer pinu w pliku [mimxrt1064_evk.dts](..\zephyrproject\zephyr\boards\nxp\mimxrt1064_evk\mimxrt1064_evk.dts) (CTRL+P -> `mimxrt1064_evk.dts` -> ENTER).
    - W sekcji `arduino_header` możemy zobaczyć że pin D5 jest przypisany do kontrolera GPIO1 pin 10. Co oznacza że w **Reference Manual** będzie oznaczony jako `GPIO1_IO10`.
    - W **Reference Manual** znajdziemy numerację pinów dla naszego kontrolera. W Tabeli 10-1 `Muxing Options` możemy znaleźć pin `GPIO1_IO10` oznaczony jako `GPIO_AD_B0_10`.

2. Sprawdzenie poprawności (czy możemy użyć tego pinu jako PWM).
    - Wyszukujemy w **Reference Manual** pin `GPIO_AD_B0_10` aż znajdziemy `SW MUX Control Register`
    - Patrzymy na dostępne opcje w polu `MUX_MODE` i sprawdzamy czy jest tam opcja `PWM` (Jest: `FLEXPWM1_PWMA03`).

`SW MUX Control Register` (Software Multiplexing Control Register) to rejestr w mikrokontrolerach, który zarządza funkcją przypisaną do danego pinu GPIO. Rejstr ten w naszym przypadku ma następującą strukturę:

| **Bit**  | **Pole**       | **Opis**                                                                  |
|----------|----------------|-------------------------------------------------------------------------  | 
| 31–5     | Reserved       | Zarezerwowane bity, muszą być ustawione zgodnie z wartością domyślną (0). |
| 4        | SION           | umożliwia ręczne wymuszanie ścieżki wejściowej, nawet jeśli wybrana funkcja tego nie wymaga. |
|          |                | `1`: Wymuszona ścieżka wejściowa.                                         | 
|          |                | `0`: Ścieżka wejściowa zgodna z funkcjonalnością MUX_MODE.                |
| 3–0      | MUX_MODE       | Wybór trybu muxowania dla pinu GPIO_AD_B0_10.                             |

3. Konfiguracja pinu w DeviceTree Overlay.
    - Robimy to dokładnie tak samo jak w ćwiczeniu z PWM (patrz [link](https://github.com/Wocck/NXP_RoboLearn/blob/lab_4_timers%26pwm/README.md)).
    - Musimy skonfigurować strukturę `pinctrl` dla naszego PWM.

---

Ten fragment definiuje konfigurację multiplexerów (MUX) dla pinu GPIO_AD_B0_10. MUX pozwala wybrać jedną z wielu funkcji przypisanych do danego pinu, np. GPIO, PWM, I2C itp. W tym przypadku wybieramy tryb FLEXPWM1_PWMA03, aby pin obsługiwał generowanie sygnału PWM. Definiujemy sobie tutaj strukturę `iomuxc_gpio_ad_b0_10_flexpwm1_pwma03_overlay` do której przypisujemy nasz pin `GPIO_AD_B0_10` oraz funkcję `FLEXPWM1_PWMA03`:
```dts
&iomuxc {
    iomuxc_gpio_ad_b0_10_flexpwm1_pwma03_overlay: IOMUXC_GPIO_AD_B0_10_FLEXPWM1_PWMA03 {
        pinmux = <0x401F80E4 0x1 0x401F8454 0x3 0x401F82D4>;
    };
};
```

**Skąd się biorą wartości `0x401F80E4 0x1 0x401F8454 0x3 0x401F82D4`?**
Wartości te pochodzą z pliku nagłówkowego fsl_iomuxc.h, znajdującego się w [fsl_iomuxc.h](..\zephyrproject\modules\hal\nxp\mcux\mcux-sdk\devices\MIMXRT1064\drivers\fsl_iomuxc.h). Gdy wyszukamy w nim nasz pin `GPIO_AD_B0_10` odnajdziemy wszystkie dostępne konfiguracje rejestru `SW MUX Control Register`. Należy pamietać aby zmienić wartości z `0x401F80E4U` na `0x401F80E4`, ponieważ w pliku dts używamy składni hexadecymalnej.

- `0x401F80E4`: Adres rejestru IOMUXC dla GPIO_AD_B0_10.
- `0x1`: Ustawienie konfiguracji pinu (np. tryb wejścia/wyjścia).
- `0x401F8454`: Adres rejestru wyboru trybu funkcji (MUX_MODE).
- `0x3`: Wartość konfiguracji MUX, odpowiadająca trybowi FLEXPWM1_PWMA03.
- `0x401F82D4`: Adres rejestru parametrów dodatkowych (np. pull-up/pull-down, prędkość sygnału).

---

Ten fragment opisuje parametry elektryczne i właściwości sygnału dla pinu GPIO_AD_B0_10. Jest to kluczowe dla zapewnienia prawidłowego działania urządzeń podłączonych do pinu.
W ramach każdej konfiguracji *pinmux* należy zdefiniować domyślną nazwę grupy pinów (`group0`). Każdy pinmux może mieć własne grupy (np. group0, group1, ...), które są niezależne od innych pinmuxów. Jest to przydatne gdy w ramach jednego pinmux kofnigurujemy wiele różnych pinów na przykład:
```dts
pinmux_spi: pinmux_spi {
    group0 {
        pinmux = <&iomuxc_gpio_ad_b0_00_spi_mosi>;
        drive-strength = "r0-4";
    };
    group1 {
        pinmux = <&iomuxc_gpio_ad_b0_01_spi_miso>;
        drive-strength = "r0-4";
    };
};
```

W naszym przypadku potrzebujemy klasycznego PWM, więc uzywamy takich samych wartośc, które możemy znaleźć w pliku `mimxrt1064_evk-pinctrl.dtsi`:
```dts
pinmux_flexpwm1: pinmux_flexpwm1 {
    group0 {
        pinmux = <&iomuxc_gpio_ad_b0_10_flexpwm1_pwma03_overlay>;
        drive-strength = "r0-4";
        bias-pull-up;
        bias-pull-up-value = "47k";
        slew-rate = "slow";
        nxp,speed = "100-mhz";
    };
};
```

Możliwe wartości parametrów możemy znaleźć w plikach *bindings* - [nxp,mcux-rt-pinctrl.yaml](..\zephyrproject\zephyr\dts\bindings\pinctrl\nxp,mcux-rt-pinctrl.yaml)

---

Na końcu musimy aktywować kontroler Flexpwm który skonfigurwaliśmy. Robi to ten fragment:
```dts
&flexpwm1_pwm3 {
    status = "okay";
    pinctrl-0 = <&pinmux_flexpwm1>;
    pinctrl-names = "default";
};
```

- `status = "okay"` - Aktywuje kontroler FLEXPWM w systemie. Bez tego linijki kontroler pozostanie nieaktywny i nie będzie generował sygnałów PWM.
- `pinctrl-0 = <&pinmux_flexpwm1>` - Przypisuje konfigurację pinów do kontrolera FLEXPWM. W naszym przypadku przypisujemy konfigurację `pinmux_flexpwm1`.
- `pinctrl-names = "default"` - Określa, że konfiguracja przypisana do pinctrl-0 jest domyślną konfiguracją (default), która zostanie załadowana przy inicjalizacji kontrolera.

--- 

Na samym końcu, aby ułatwić odwoływanie się do PWM w programie, stosując aliasy. Używamy do tego istniejącą w dts strukturę pwmleds aby mieć wszystkie PWM w jednym miejscu:
```dts
/ {
    pwmleds {
        d5_pwm: d5_pwm {
            pwms = <&flexpwm1_pwm3 0 0 PWM_POLARITY_NORMAL>;
        };
    };
};
```
Atrybut pwms wskazuje kontroler PWM oraz jego konfigurację:
- Pierwsza liczba: Kanał PWM w kontrolerze (np. 0).
- Druga liczba: Okres sygnału PWM (np. 0 oznacza domyślną wartość).
- Trzecia liczba: Polaryzacja sygnału PWM (PWM_POLARITY_NORMAL oznacza standardową polaryzację).

Alias ułatwia odwoływanie się do urządzeń zdefiniowanych w pwmleds.`pwm-d5` to alias, który wskazuje na d5_pwm zdefiniowany w sekcji `pwmleds`.
Alias pozwala na łatwiejsze odwołanie w kodzie aplikacji, np. za pomocą makr DT_ALIAS(pwm_d5):

```dts
/ {
    aliases {
        pwm-d3 = &d3_pwm; /* PWM na D3 (GPIO_AD_B1_08) */
        pwm-d5 = &d5_pwm; /* PWM na D5 (GPIO_AD_B0_10) */
    };
};
```

---

## Etap 2: Implementacja programu

W tej części napiszemy program obsługujący serwomechanizm oraz czujnik HC-SR04. Celem jest połączenie działania obu urządzeń w jednym programie, zdefiniowanie struktury danych do przesyłania oraz integracja tej funkcjonalności z nadajnikiem NRF24 za pomocą `ack payload`.

Aby uprościć implementację, możemy skorzystać z funkcji napisanych w poprzednich zajęciach. W szczególności biblioteka `hcsr04.h` zawiera funkcje umożliwiające pomiar odległości, które należy zintegrować z obsługą serwomechanizmu. Serwomechanizm musi pracoać w pętli (od lewej do prawej) aby umożliwić czujnikowi odległości zebranie pomiarów w zakresie 180 stopni. 
Dane odczytane z czujnika i ustawienia serwomechanizmu muszą zostać zapakowane w strukturę danych, która będzie przesyłana do nadajnika NRF24 w polu ack payload. Możemy zdefiniować własną strukturę lub skorzystać z gotowego przykładu dostępnego [tutaj](https://github.com/Wocck/NXP_RoboLearn/blob/esp32_remote_control/remote_lidar.ino).

Aby przesyłać dane w polu ack payload, należy dodać nową funkcję w bibliotece nrf24.h, która obsłuży naszą strukturę. Przykład implementacji:
```C++
int send_ack_payload(const uint8_t* data, size_t length);
```

Wszystkie funkcje należy zintegrować w głównym programie (main.c), korzystając z wątków k_thread w systemie Zephyr RTOS.
Domyślnie zakładamy 3 wątki do obsługi odbiornika radiowego, silników oraz lidara. 