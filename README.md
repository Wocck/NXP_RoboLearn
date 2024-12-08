# NXP_RoboLearn
This is simple platform for learning NXP evaluation board to make a Robot

## 1. Timery w Zephyr RTOS

Timery są nieodłącznym elementem systemów wbudowanych, umożliwiając wykonywanie zadań w określonych odstępach czasu lub po upływie określonego czasu. W Zephyr RTOS dostępne są różne mechanizmy do pracy z timerami, które pozwalają na precyzyjne zarządzanie czasem w aplikacjach.

Timery możemy podzielić na dwa rodzaje:
1. **Timery programowe** – są to timery, które są emulowane w oprogramowaniu. Timery programowe są mniej precyzyjne niż timery sprzętowe, ale są bardziej elastyczne i można ich używać w większej liczbie.
2. **Timery sprzętowe** – są to timery wbudowane w mikrokontroler, które są zazwyczaj wykorzystywane do obsługi zdarzeń czasowych w systemie. Timery sprzętowe są zazwyczaj bardziej precyzyjne niż timery programowe, ale ich liczba jest ograniczona.

### 1.1 Timery programowe

Przykłądowy program demonstrujący działanie timera *Programowego* znajduje się w `LED_timer`.
Dokładny opis API Zephyra odpowiadjący za obsługę timerów znajduje się w [dokumentacji](https://docs.zephyrproject.org/latest/kernel/services/timing/timers.html).

Aby użyć timera musimy wykonać kilka kroków:
1. Zdefiniowanie timera
```C++
static struct k_timer my_timer;
```

2. Zainicjalizowanie timera
```C++
k_timer_init(&my_timer, timer_expiry_function, timer_stop_function);
```

3. Uruchomienie timera
```C++
k_timer_start(&my_timer, K_MSEC(1000), K_MSEC(2000));
```
- `K_MSEC(1000)` – czas do pierwszego wywołania w milisekundach.
- `K_MSEC(2000)` – okres powtarzania w milisekundach (lub K_NO_WAIT, jeśli nie chcesz powtarzać).

4. Zatrzymanie timera
```C++ 
k_timer_stop(&my_timer);
```

**Ważne uwagi dotyczące używania timerów**
- *Kontekst wykonania funkcji zwrotnych*: Funkcje wywoływane przez timer są wykonywane w kontekście systemowego wątku obsługi timerów. Należy unikać operacji blokujących i długotrwałych.
- *Synchronizacja*: Jeśli funkcja zwrotna modyfikuje wspólne zasoby, konieczne jest zapewnienie synchronizacji (np. poprzez mutexy).
- *Dokładność*: Timery programowe nie są tak precyzyjne jak timery sprzętowe. Dla krytycznych zastosowań czasowych warto rozważyć użycie timerów sprzętowych.

### 1.2 Timery sprzętowe

Timer sprzętowy to komponent pozwalający na precyzyjne odmierzanie czasu i generowanie zdarzeń w systemie embedded. Płyta MIMXRT1064-EVK wyposażona jest w kilka rodzajów timerów, takich jak General Purpose Timer (GPT), Periodic Interrupt Timer (PIT) oraz Quad Timer (TMR).

- **General Purpose Timer (GPT)** – jest to uniwersalny, 32-bitowy timer, który może pracować w różnych trybach (m.in. normalnym, wejściowym, wyjściowym czy PWM) oraz posiadać rozbudowane opcje preskalera. Duży zakres licznikowy pozwala na łatwe uzyskiwanie długich okresów, np. rzędu sekund, bez potrzeby stosowania trików programowych.

- **Periodic Interrupt Timer (PIT)** – to 32-bitowy timer zaprojektowany do generowania okresowych przerwań, często taktowany zegarem IPG (np. 75 MHz lub 150 MHz, w zależności od konfiguracji). Dzięki dużej rozdzielczości (32 bity) można łatwo uzyskać długie interwały między przerwaniami. PIT nie posiada wewnętrznego preskalera, jednak przy tak dużej przestrzeni zliczania (0xFFFFFFFF) można bezpośrednio ustawić liczbę tików odpowiadającą pożądanemu czasowi. Na przykład, przy częstotliwości 75 MHz, ustawienie top na wartość 75 000 000 zapewni przerwanie co 1 sekundę. PIT jest w pełni obsługiwany przez API `counter` w Zephyrze, co znacznie upraszcza jego użycie.

- **Quad Timer (TMR)** – to moduł 16-bitowy z czterema niezależnymi licznikami. Chociaż jest bardzo elastyczny (np. generowanie PWM, zliczanie impulsów, pomiar szerokości impulsów), ograniczenie do 16 bitów powoduje, że maksymalna wartość zliczania wynosi 65535. W praktyce utrudnia to bezpośrednie uzyskanie długich interwałów (np. dokładnie 1 sekundy przy wysokich częstotliwościach zegara) i wymaga dzielenia czasu na mniejsze odcinki oraz re-armowania alarmu w callbacku. QTMR jest jednak często wybierany do prostych zadań ze względu na gotowe wsparcie w Zephyrze i łatwość konfiguracji – przy krótkich czasach i niższych częstotliwościach sprawdza się świetnie.

W naszym przypadku skorzystamy z **PIT**, gdyż jego 32-bitowy zakres i integracja z API `counter` w Zephyrze umożliwiają proste i bezpośrednie ustawienie dłuższych interwałów bez konieczności dzielenia czasu na mniejsze odcinki czy stosowania trików.

Jeśli jednak potrzebowalibyśmy stosować QTMR i uzyskać np. dokładnie 1 sekundę przy wysokiej częstotliwości zegara, musielibyśmy podzielić tę sekundę na kilka krótszych interwałów. Po każdym przerwaniu (wywołaniu callbacku) ponownie ustawialibyśmy alarm na kolejny mniejszy odcinek czasu. W efekcie, zliczając liczbę krótkich interwałów w callbacku, po osiągnięciu sumarycznie 1 sekundy zmienialibyśmy stan diody. Taki mechanizm jest konieczny, ponieważ QTMR nie pozwala bezpośrednio na tak długi interwał z powodu ograniczeń 16-bitowego zakresu zliczania oraz maksymalnej wartości prescalera równej 128.

### 1.3 Konfiguracja PIT w Zephyrze

1. W pliku `.overlay` musimy włączyć PIT, aby mógł być używany w naszym projekcie:
```dts
&pit0_channel0 {
    status = "okay";
};
```
2. W programie musimy zdefiniować strukturę `counter_top_cfg`:
```C++
struct counter_top_cfg cfg = {
    .ticks = 1000,                      // Ustaw granicę zliczania do 1000 ticków
    .callback = my_callback_function,   // Funkcja wywoływana przy przekroczeniu granicy zliczania
    .user_data = NULL,                  // Wskaźnik na dane użytkownika, które zostaną przekazane do funkcji callback.
    .flags = 0                          // Domyślne zachowanie cykliczne (wartość licznika jest resetowana do zera po przekroczeniu granicy)
};
```

3. Następnie należy zainicjować i uruchomić licznik:
```C++
// Zainicjuj strukturę PIT
if (counter_set_top_value(pit_ch0, &cfg) < 0) {
    printk("Failed to set top value\n");
    return -1;
}

// Uruchom timer PIT
if (counter_start(pit_ch0) < 0) {
    printk("Failed to start PIT\n");
    return -1;
}
```

Przykładowy kod znajduje się w `LED_hardware_timer`.

---


## 2. PWM z użyciem timera w Zephyr RTOS

Zephyr oferuje spójne i proste w użyciu API do obsługi PWM, co pozwala łatwo sterować jasnością diody poprzez zmianę współczynnika wypełnienia (duty cycle) sygnału PWM. Dokumentacja dostępna jest w [Zephyr PWM API](https://docs.zephyrproject.org/latest/reference/peripherals/pwm.html).

Aby używać PWM w Zephyrze, należy wykonać następujące kroki:

1. **Definicja urządzenia PWM w Device Tree (DTS)**  
Ten krok możemy pominąć ponieważ domyślna struktura `mimxrt1064_evk.dts` zawiera już konfigurację PWM dla diody LED.

2. **Inicjalizacja urządzenia PWM**

```C++
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
```

3. **Ustawienie częstotliwości i współczynnika wypełnienia**

```C++
uint32_t period_ns = 1000000; // 1 ms = 1 kHz
uint32_t pulse_ns = 500000;   // 0.5 ms = 50% duty cycle

if (pwm_set_dt(&pwm_led0, period_ns, pulse_ns)) {
    printk("Failed to set PWM\n");
    return -1;
}
```

Powyższy przykład ustawia częstotliwość na `1 kHz` oraz współczynnik wypełnienia na `50%`, co skutkuje średnią jasnością diody.

4. **Dynamiczna zmiana jasności (duty cycle)**

Aby płynnie rozjaśniać i przyciemniać diodę, wystarczy cyklicznie zmieniać pulse_ns i ponownie wywoływać `pwm_set_dt()` z nową wartością.
```C++
// Stopniowo zwiększaj wypełnienie od 0% do 100%, a potem zmniejszaj do 0% w pętli
uint32_t step = period_ns / 100;
uint32_t duty = 0;
bool increasing = true;

while (true) {
    if (pwm_set_dt(&pwm_led0, period_ns, duty)) {
        printk("Failed to update PWM duty\n");
    }

    k_sleep(K_MSEC(10)); // krótkie opóźnienie dla płynności

    if (increasing) {
        duty += step;
        if (duty >= period_ns) {
            duty = period_ns;
            increasing = false;
        }
    } else {
        duty -= step;
        if (duty < step) {
            duty = 0;
            increasing = true;
        }
    }
}
```

**Ważne uwagi dotyczące używania PWM**:

- *Wybór pinu*: Upewnij się, że pin, na którym generujesz PWM, jest obsługiwany przez wybrany kontroler PWM i poprawnie zdefiniowany w DTS.
Prędkość zmian: Zbyt częsta aktualizacja duty cycle może zużywać zasoby i prowadzić do migotania diody. Zwykle aktualizacja co kilkanaście milisekund daje płynny efekt.
- *Dokładność*: Dokładność i stabilność sygnału PWM zależy od zegara i możliwości sprzętowego kontrolera.
- *Zastosowania*: PWM może być używane do sterowania jasnością LED, prędkością silników czy pozycją serwomechanizmów, pozwalając na płynne i precyzyjne sterowanie bez konieczności stosowania analogowych sygnałów napięciowych.


## **L298N** - DC Motor Driver

Teraz spróbujemy oprogramować moduł L298, który jest mostkiem H do sterowania silnikami prądu stałego. Moduł L298 pozwala na sterowanie dwoma silnikami DC w obie strony (przód/tył) oraz regulację prędkości. W naszym przypadku wykorzystamy moduł L298N, który jest popularnym i łatwym w użyciu mostkiem H.

**Wyprowadzenia modułu L298N:**
| Pin   | Opis                                                                                           |
|-------|------------------------------------------------------------------------------------------------|
| +12V  | Napięcie zasilania silników.                                                                   |
| +5V   | Zasilanie części logicznej za stabilizatorem - aktywne po wyjęciu zworki 5V.                   |
| GND   | Masa układu                                                                                    |
| OUT1, OUT2 | Wyjścia kanału silnika A.                                                                 |
| OUT3, OUT4 | Wyjścia kanału silnika B.                                                                 |
| ENA   | Sygnał PWM do sterowania prędkością obrotową silnika A. Domyślnie podłączony do 5 V za pomocą zworki - oznacza, że silnik pracuje z maksymalną prędkością. |
| IN1, IN2 | Sterowanie kierunkiem kanału A                                                              |
| ENB   | Sygnał PWM do sterowania prędkością obrotową silnika B. Domyślnie podłączony do 5 V za pomocą zworki - oznacza, że silnik pracuje z maksymalną prędkością. |
| IN3, IN4 | Sterowanie kierunkiem kanału B                                                              |

**Sterowanie:**
| IN1 / IN3 | IN2 / IN4 | Wyjścia silników |
|-----------|-----------|------------------|
| wysoki    | niski     | Silnik kręci się z maksymalną prędkością zadaną poprzez PWM (różną od 0) zgodnie ze wskazówkami zegara. |
| niski     | wysoki    | Silnik kręci się z maksymalną prędkością zadaną poprzez PWM (różną od 0) przeciwnie do ruchu wskazówek zegara. |
| niski     | niski     | Przy podaniu stanu wysokiego na wejście PWM - szybkie hamowanie silników (fast stop). |
| wysoki    | wysoki    | Przy podaniu stanu wysokiego na wejście PWM - szybkie hamowanie silników (fast stop). |
| wysoki    | wysoki    | Przy podaniu stanu niskiego na wejście PWM - swobodne hamowanie (soft stop). |


Przykładowe połączenie pinów płytki `MIMXRT1064_evk` z modułem `L298N`:

| Pin na płytce MIMXRT1064-EVK  | Pin na module L298N | Opis                    |
|-------------------------------|---------------------|-------------------------|
| D4  (GPIO_AD_B0_09)           | ENB                 | PWM dla silnika B       |
| D3  (GPIO_AD_B1_08)           | ENA                 | PWM dla silnika A       |
| D2                            | IN1                 | Sterowanie kierunkiem   |
| D5                            | IN2                 | Sterowanie kierunkiem   |
| D6                            | IN3                 | Sterowanie kierunkiem   |
| D7                            | IN4                 | Sterowanie kierunkiem   |


## **Konfiguracja PWM na pinach arduino**

Aby było łatwiej użyjemy już skonfigurowanego pinu PWM, ktrórego używaliśmy do generowania sygnału pwm na wbudowaną diodę led ponieważ jest on jednocześnie podłączony do pinu D4. Możemy to wyczytać z *reference manual* płytki MIMXRT1064-EVK oraz plików `dts` i `pinctrl.dtsi`

- Pin `GPIO_AD_B0_09` jest przypisany do `GPIO1` (`GPIO1_IO09`) co możemy wyczytać w tabeli *Table 10-1. Muxing Options*.
- W tej samej tabeli możemy wyczytać, że pin jest również przypisany do `FLEXPWM2_PWM3_A`

Możemy sprawdzić czy istnieje konfiguracja w strukturze `dts` dotycząca tego PWM. W pliku `mimxrt1064_evk.dts` znajdujemy:

```dts
aliases {
		pwm-led0 = &green_pwm_led;
        ...
	};

pwmleds {
    compatible = "pwm-leds";

    green_pwm_led: green_pwm_led {
        pwms = <&flexpwm2_pwm3 0 PWM_MSEC(20) PWM_POLARITY_NORMAL>;
    };
};

&flexpwm2_pwm3 {
	status = "okay";
	pinctrl-0 = <&pinmux_flexpwm2>;
	pinctrl-names = "default";
};
```

Powyższa struktura opisuje konfigurację PWM dla diody LED. Możemy zauważyć, że PWM jest skonfigurowane na `flexpwm2_pwm3` i przypisane do `GPIO1_IO09` czyli pinu arduino `<10 0 &gpio1 9 0>,	/* D4 */`. Możemy więc użyć tego PWM do sterowania silnikiem. PWM który podamy na ten pin będzie sterował zarówno diodą jak i pinem `D4`.

Następnie potrzbujemy drugiego PWM, który będzie sterował drugim silnikiem. Aby znaleźć taki pin skorzystamy z tabeli *Table 10-1. Muxing Options* w *reference manual* płytki MIMXRT1064-EVK. Znajdujemy, że pin `GPIO_AD_B1_08` jest przypisany do `FLEXPWM4_PWM0_A`. Jest to pin `GPIO1_IO24` czyli `<9 0 &gpio1 24 0>,	/* D3 */`. Jeżeli spojrzymy w struktury `dts` nie znajdziemy jednak przypisanego tego pinu do kontrolera PWM. Musimy to skonfigurować ręcznie w pliku overlay.

1. Odniesienie do węzła kontrolera pinów w Device Tree.

```dts
&pinctrl {
    /* Konfiguracja pinmux dla GPIO_AD_B1_08 */
    pinmux_flexpwm4: pinmux_flexpwm4 {
        group0 {
            pinmux = <&iomuxc_gpio_ad_b1_08_flexpwm4_pwma0_overlay>;
            drive-strength = "r0-4";
            bias-pull-up;
            bias-pull-up-value = "47k";
            slew-rate = "slow";
            nxp,speed = "100-mhz";
        };
    };
};
```

2. Konfiguracja samego pinu na funkcję `ALT1`
Konkretne wartości rejestrów możemy znaleźć w pliku `fsl_iomuxc.h` przez wyszukanie w nim pinu `GPIO_AD_B1_08`.
Znajdziemy w nim wartości rejestrów dla różnych funckji pinu. Nas interesuje funkcja `ALT1` czyli `FLEXPWM4_PWMA00`.

`ALT1 — Select mux mode: ALT1 mux port: FLEXPWM4_PWMA00 of instance: flexpwm4`:
```C++
#define IOMUXC_GPIO_AD_B1_08_FLEXSPIA_SS1_B 0x401F811CU, 0x0U, 0, 0, 0x401F830CU
#define IOMUXC_GPIO_AD_B1_08_FLEXPWM4_PWMA00 0x401F811CU, 0x1U, 0x401F8494U, 0x1U, 0x401F830CU
#define IOMUXC_GPIO_AD_B1_08_FLEXCAN1_TX 0x401F811CU, 0x2U, 0, 0, 0x401F830CU
```

Przepisujemy wartości rejestrów do pliku `.overlay`:
```dts
&iomuxc {
    iomuxc_gpio_ad_b1_08_flexpwm4_pwma0_overlay: IOMUXC_GPIO_AD_B1_08_FLEXPWM4_PWMA00 {
        pinmux = <0x401F811C 0x1 0x401F8494 0x1 0x401F830C>;
    };
};
```

3. Aktywacja węzła `flexpwm4_pwm0` i przypisanie aliasu 

```dts
&flexpwm4_pwm0 {
    status = "okay";
    pinctrl-0 = <&pinmux_flexpwm4>;
    pinctrl-names = "default";
};
```

Ta sekcja definiuje urządzenie (d3_pwm) sterowane przez PWM4. Określa, który kontroler PWM, kanał oraz parametry sygnału PWM mają być używane do sterowania:

```dts
/ { 
    pwmleds {
        d3_pwm: d3_pwm {
            pwms = <&flexpwm4_pwm0 0 PWM_MSEC(20) PWM_POLARITY_NORMAL>;
        };
    };
};
```

Tu definiujemy alias, który umożliwia aplikacji łatwe odniesienie się do konfiguracji PWM4 dla D3.

```dts
/ {
    aliases {
        pwm-d3 = &d3_pwm; /* PWM na D3 (GPIO_AD_B1_08) */
    };
};
```

---

Dzięki tej konfiguracji w `overlay` możemy używać pwm dla pinu D3 tak samo jak robiliśmy to z wbudowaną diodą led. Możemy teraz użyć tego PWM do sterowania drugim silnikiem.

```C++
static const struct pwm_dt_spec pwm_d4_spec = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
static const struct pwm_dt_spec pwm_d3_spec = PWM_DT_SPEC_GET(DT_ALIAS(pwm_d3));
```

---

## ** Sterowanie silnikami DAC przy pomocy PWM**

Prędkość silnika jest kontrolowana poprzez PWM na pinach ENA (dla silnika A) i ENB (dla silnika B). PWM pozwala na płynną regulację mocy dostarczanej do silnika poprzez zmienianie wypełnienia impulsów.
Wypełnienie PWM (Duty Cycle): Określa, jak długo sygnał PWM jest w stanie HIGH w cyklu PWM:
- Wyższe wypełnienie (bliżej 100%): Większa moc i prędkość silnika.
- Niższe wypełnienie (bliżej 0%): Mniejsza moc i prędkość silnika.

Częstotliwość PWM to liczba cykli sygnału PWM na sekundę. Jest odwrotnością okresu (period_ns). W dokumentacji sterownika znajdziesz zakres częstotliwości, z jakimi może pracować. Większość sterowników obsługuje częstotliwości od 1 kHz do 20 kHz. Częstotliwości poniżej 20 kHz mogą generować piski słyszalne dla ludzkiego ucha więc najlepiej zastosowac wyższą częstotliwość. 

Wypełnienie PWM (ang. duty cycle) określa procentowy czas, przez jaki sygnał jest w stanie HIGH w ramach jednego cyklu. Jest to kluczowy parametr wpływający na prędkość i moment obrotowy silnika. Silnik musi pokonać pewne fizyczne ograniczenia więc należy znaleźć minimalne wypełnienie, które pozwoli na uruchomienie silnika. 

---

## **Ćwiczenie 1: Znajdź minimalne wypełnienie**

1. **Podłącz silnik do modułu L298N** zgodnie z opisem w sekcji "Przykładowe połączenie pinów płytki `MIMXRT1064_evk` z modułem `L298N`".
2. **Skonfiguruj PWM** na pinach D3 i D4 zgodnie z instrukcjami w sekcji "Konfiguracja PWM na pinach arduino".
3. **Uruchom program** sterujący PWM na pinie D4 (ENA) i D3 (ENB) z minimalnym wypełnieniem (np. 10%).
4. **Zwiększaj wypełnienie** PWM co 10% i obserwuj, przy jakim wypełnieniu silnik zaczyna się obracać.
5. **Zanotuj minimalne wypełnienie** PWM, przy którym silnik zaczyna się obracać.

## **Ćwiczenie 2: Sterowanie za pomocą Joysticka**

Stwórz metody do sterowania prędkością silnika A i B za pomocą joysticka. Joystick wysyła dane z zakresu <-90, 90>, które musisz przekształcić na wypełnienie PWM. Przykładowo, jeśli joystick wysyła 0 stopni, to silnik powinien być zatrzymany. Jeśli joystick wysyła 90 stopni, to silnik powinien obracać się z maksymalną prędkością w jedną stronę, a dla -90 stopni w drugą stronę.

---
