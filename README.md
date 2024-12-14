# NXP_RoboLearn

## Przerwania w Zephyr RTOS

W Zephyr RTOS przerwania GPIO są obsługiwane za pomocą API GPIO. Poniżej znajduje się opis kroków potrzebnych do skonfigurowania przerwań GPIO oraz szczegóły działania używanych funkcji.

---

### Krok 1: Definicja pinu GPIO w Device Tree

W Device Tree należy upewnić się, że kontroler GPIO oraz pin są poprawnie skonfigurowane. Przykład dla GPIO5 i pinu 0:

- Konfiguracja w pliku `mimxrt1064_evk.dts` mówi nam który kontroler gpio odpowiada za obsługę przycisku oraz do którego pinu jest przypisany: 
```dts
gpio_keys {
    compatible = "gpio-keys";
    user_button: button-1 {
        label = "User SW8";
        gpios = <&gpio5 0 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>;
        zephyr,code = <INPUT_KEY_0>;
    };
};
```

- Następnie po zbudowaniu projektu możemy przeszukać plik `build\zephyr\zephyr.dts` pod kątem kontrolera `gpio5`. W tym pliku znajduje się pełna konfiguracja DeviceTree dla naszego urządzenia:
```dts
gpio5: gpio@400c0000 {
    compatible = "nxp,imx-gpio";
    reg = < 0x400c0000 0x4000 >;
    interrupts = < 0x58 0x0 >, < 0x59 0x0 >;
    gpio-controller;
    #gpio-cells = < 0x2 >;
    pinmux = < &iomuxc_snvs_wakeup_gpio5_io00 >, < &iomuxc_snvs_pmic_on_req_gpio5_io01 >, < &iomuxc_snvs_pmic_stby_req_gpio5_io02 >;
    phandle = < 0x17d >;
};
```
W tej części istotnym dla mas kodem jest fragment `interrupts = < 0x58 0x0 >, < 0x59 0x0 >;` definiuje on dwa numery przerwań (`0x58` oraz `0x59`). Są to to unikalne identyfikatory pozwalające procesorowi rozpoznać, które urządzenie wywołało przerwanie. Często są stosowane jako indeksy w wektorze obługi przerwań procesora, które wskazują na funkcjie ISR obsługujące dane przerwanie. Oznacza to że kontroler GPIO5 obsługuje dwa różne przerwania, z których każde może być przypisane do konkretnego pinu lub grupy pinów w ramach kontrolera GPIO.

W `Reference Manual` do płytki `mimxrt1064_evk` możemy wyczytać, który numer przerwania odpowiada naszemu przyciskowi (`&gpio5 0`).

![Domain Interrupt Summary](images/domain_interrupt_summary.png)

Jak widać w tabeli do pinu 0 kontrolera *GPIO5* używany jest numer 88.

### Krok 2: Pobranie kontrolera GPIO i skonfigurowanie jako wejście

Dla uproszczenia kodu korzystamy z struktur devicetree (DT) zdefiniowanych w `mimxrt1064_evk.dts`:

```C++
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_NODELABEL(user_button), gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(green_led), gpios);

int ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
```

### Krok 3: Rejestracja przerwania GPIO

Konfigurujemy przerwanie w następujący sposób:
- `GPIO_INT_EDGE_TO_ACTIVE` - przerwanie wywoływane jest gdy stan pinu zmienia się z niskiego na wysoki

Funkcja "pod spodem" ustawia rejestry sprzętowe w celu wykrywania określonego zbocza, włącza maskę przerwań dla pinu przycisku oraz rejestruje przerwanie w systemie Zephyr. Wszystko to dzieje się automatcznie dzięki API kontrolera GPIO.
```C++
ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
```

### Krok 4: Zdefiniowanie ISR obsługującej przerwanie

Aby nasze przerwanie "coś" robiło musimy zdefiniować funckcję obsługi przerwania czyli **Interrupt Service Routine**. W implementacji używa się określenia *callback*. Powinna to być szybka i zoptymalizowana funkcja aby zapewnić responsywność systemu. W naszym przykładzie będzie to funckja zmieniająca stan diody led za pomocą `gpio_pin_toggle_dt()` oraz wypisująca informację w konsoli.

**Uwaga** Jak pamiętamy z poprzednich skryptów funkcje wypisujące do konsoli są bardzo kosztowne da procesora dlatego nidy nie powinno się ich używać w callbackach przerwań. W naszym przypadku jest to jedynie cel demonstracyjny.

```C++
static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    gpio_pin_toggle_dt(&led);
    printk("Przycisk wciśnięty! Zmieniam stan LED\n");
}
```

### Krok 5: Rejestracja callbacku

Gdy już mamy skonfigurowany pin z przerwaniem oraz zdefiniowaną funkcję obsługi przerwania, możemy zarejestrować callback w systemie Zephyr. Używamy do tego funkcji `gpio_init_callback()` oraz `gpio_add_callback()`.

```C++
gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
gpio_add_callback(button.port, &button_cb_data);
```

### Podsumowanie

Po tych krokach możemy uruchomić nasz program. W pętli głównej uśpimu proces głowny aby nie robił nic i czekał na wystąpienie przerwania za pomocą funckji `k_sleep()`. W momencie wciśnięcia przycisku dioda LED powinna zmienić stan oraz w konsoli powinna pojawić się informacja o wciśnięciu przycisku.

```C++
while (1) {
    k_sleep(K_FOREVER); // Proces główny pozostaje w uśpieniu
}
```

---

## Debouncing w przerwaniach GPIO

### Drgania styków przycisków

Gdy przycisk jest wciskany, styki wewnątrz przycisku mogą wielokrotnie odbijać się od siebie, zanim osiągną stabilny stan. To zjawisko nazywane jest drganiami styków (ang. bounce). W efekcie, zamiast jednego przejścia z niskiego stanu logicznego do wysokiego (lub odwrotnie), możemy zaobserwować serię szybkich przejść, które mogą być błędnie interpretowane przez system jako wielokrotne naciśnięcia przycisku.

Bez odpowiedniego debouncingu, system może reagować na każde z tych szybkich przejść, co prowadzi do niepożądanych efektów, takich jak wielokrotne wywołanie przerwania lub wykonanie akcji przypisanej do przycisku.

![Bounce Przycisku](images/bounce.png)

### Jak działa debouncing?

Debouncing można realizować na dwa sposoby: sprzętowo i programowo.

1. **Debouncing sprzętowy**

Debouncing sprzętowy polega na użyciu dodatkowych komponentów elektronicznych, takich jak kondensatory i rezystory, które filtrują szybkie zmiany sygnału, pozwalając tylko na stabilne przejścia. Jest to skuteczna metoda, ale wymaga dodatkowych elementów na płytce drukowanej.

2. **Debouncing programowy**

Debouncing programowy polega na implementacji algorytmu w kodzie, który ignoruje szybkie zmiany sygnału w krótkim okresie czasu. Przykładowo, po wykryciu zmiany stanu przycisku, program może odczekać określony czas (np. 10-50 ms) i ponownie sprawdzić stan przycisku, aby upewnić się, że jest stabilny. Dopiero wtedy zmiana stanu jest uznawana za prawidłową.

Poniżej schemat blokowy takiego rozwiązania:

![Debouncing schemat](images/debounce_diagram.png)


## **Ćwiczenie 1**

W projekcie `Button_interrupt` zaimplementuj mechanizm debouncingu. Zdefinuj stałą zmienną `DEBOUNCE_DELAY_MS` i użyj jej w procedurze obsługi przerwania. Skorzystaj z funkcji zephyra do pomiaru czasu:

```C++
k_uptime_get_32(); // Pobierz czas w ms
```

---

## **Moduł NRF24L01 i przerwania**

Wróćmy teraz do implementacji komunikacji szeregowej z modułem NRF24L01. Na poprzednim laboratorium korzystaliśmy z ciągłego odpytywania moduły w pętli głównej czy pojawiły się jakieś nowe dane. Działało to w ten sposób:
1. Odczytanie wartości rejestru `STATUS (0x07)`
2. Sprawdzenie czy flaga `RX_DR` w rejestrze jest ustawiona (czy nowe dane się pojawiły)
3. Jeżeli tak to wysyłamy komendę `R_RX_PAYLOAD (0x61)` aby pobrać dane z bufora odbiorczego
4. Po odebebraniu danych przez interfejs SPI ustawiamy flagę `RX_DR` w rejestrze `STATUS` na 1 aby ją zresetować
Następnie powtarzamy cały proces ponieważ metoda wywoływana jest w pętli. 

Jeżeli zajrzymy do dokumentacji modułu NRF24L01 to znajdziemy fragment opisujący jak powinniśmy obsłóżyć przerwania:
```
The RX_DR IRQ is asserted by a new packet arrival event. The procedure for handling this interrupt should be: 
1) Read payload through SPI. 
2) Clear RX_DR IRQ.
3) Read FIFO_STATUS to check if there are more payloads available in RX_FIFO.
4) If there are more data in RX FIFO, repeat from step 1).
```
Oznacza to że w naszym poprzednim programie poprawnie obsługiwaliśmy flagę RX_DR a nie musieliśmy sprawdzać statusu rejestru FIFO_STATUS, ponieważ nie było ryzyka przepełnienia gdy cały czas odczytywaliśmy dane z bufora odbiorczego.

W przerwaniach będziemy musieli do tego podejść innaczej tak jak jest to opisane w dokumentacji. To znaczy że dla każdego przerwania będziemy musieli w pętli sprawdzać czy kolejka FIFO zawiera jeszcze jakieś dane i jeżeli tak to powinniśmy je odczytać. 

### Krok 1: Podłączenie pinu IRQ do pinu GPIO

Pierwszym krokiem jest podłączenie pinu IRQ modułu NRF24L01 do pinu GPIO naszego mikrokontrolera. W naszym przypadku będzie to pin `D8` (GPIO1 3).

![NRF24L01 Pinout](images/nRF24L01.png)

### Krok 2: Konfiguracja przerwania GPIO 

Aby poprawnie skonfigurować pin GPIO w naszym mikrokontrolerze musimy dowiedzieć się z dokumentacji jak działa pin IRQ modułu radiowego. Oto najważniejsze informacje:
- Pin jest typu `Open-Drain Output` (innaczej open collector).
- Pin jest domyślnie w stanie wysokim i gdy moduł odbierze dane ściąga pin do stanu niskiego na około 100us.
- Ponieważ pin jest *Open-Drain* wymaha użycia rezystora `Pull-Up` aby zapewnić że linia IRQ zostanie podciągnięta do stanu wysokiego kiedy moduł nie ściąga jej do stanu niskiego.

Biorąc pod uwagę powyższe informacje nasz pin gpio należy skonfigurować w następujący sposób:
```C++
int ret = gpio_pin_configure(irq_dev, IRQ_GPIO_PIN, GPIO_INPUT | GPIO_PULL_UP);
ret = gpio_pin_interrupt_configure(irq_dev, IRQ_GPIO_PIN, GPIO_INT_EDGE_TO_INACTIVE);
```
Flaga `GPIO_INT_EDGE_TO_INACTIVE` sprawia że przerwanie zostanie wywołane gdy stan pinu zmieni się z wysokiego na niski.

### Krok 3: Zdefiniowanie ISR obsługującej przerwanie

Jak zobaczyliśmy w dokumentacji modułu nasza funckja obsługujaca przerwanie powinna wyglądać mniej więcej tak:

```C++
static void irq_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    printk("IRQ pin triggered!\n"); // nie używaj printk w ISR!!

    while (true) {
        // Sprawdź status rejestru FIFO
        uint8_t fifo_status;
        if (radio.read_register(0x17, &fifo_status, 1) != 0) { // FIFO_STATUS register
            printk("Failed to read FIFO_STATUS register\n");
            break;
        }

        if (fifo_status & 0x01) { // RX_EMPTY bit
            // Jeśli FIFO RX jest puste, kończymy obsługę przerwania
            break;
        }

        // Jeśli FIFO nie jest puste, odbierz dane
        struct DataPacket packet;
        if (radio.receive_payload(&packet) == 0) {
            printk("Received packet: joystickX=%d, joystickY=%d, buttonPressed=%d\n",
                   packet.joystickX, packet.joystickY, packet.buttonPressed);
        } else {
            printk("Failed to receive payload\n");
        }

        // Wyczyść flagę RX_DR w STATUS
        uint8_t clear_flags = 0x40; // RX_DR
        radio.write_register(0x07, &clear_flags, 1);
    }
}
```

## **Ćwiczenie 2**

W projekcie `NRF24L01_example` zaimplementuj obsługę przerwań dla modułu NRF24L01. Skonfiguruj pin IRQ jako wejście z rezystorem Pull-Up oraz zdefiniuj funkcję obsługi przerwania. Zainicjuj i dodaj callback do pinu GPIO tak jak w **Ćwiczenie 1**. W funkcji obsługi przerwania odczytaj dane z bufora odbiorczego i wypisz je w konsoli. Sprawdź czy kod działa poprawnie i zapisz wyniki...

---

## Dlaczego nie działa?

Prawdopodonie w trakcie testów zauważyłeś że przerwania nie są obsługiwane poprawnie. W konsoli nie pojawiają się informacje o odebraniu pakietu. Wynik który otrzymaliśmy prawdopodobnie wyglądał tak:
```
IRQ pin triggered!
Failed to read FIFO_STATUS register
```

Powodem dla którego kod nie zadziałał jest nasz ISR w ktorym przeprowadzamy zabronione działania. ISR powinna spełniać następujące warunki:
1. Funkcja obsługi przerwania musi być szybka (krótka)
2. W ISR nie można bezpośrednio korzystać z operacji wymagających synchronizacji lub dostępu do zasobów współdzielonych, takich jak na przykład SPI.

Wywołanie operacji SPI (radio.read_register) w ISR powodowało, że sterownik SPI nie mógł poprawnie zsynchronizować swojego działania, co prowadziło do błędów, takich jak brak odpowiedzi modułu nRF24L01+. Sterownik SPI w Zephyrze może wymagać:
- Przerwań do obsługi swojej komunikacji – te przerwania są blokowane, gdy ISR działa.
- Mutexów lub semaforów do synchronizacji – mechanizmy te nie działają w ISR.

### Rozwiązanie

Rozwiązaniem powyższego problemu jest mechanizm **Workqueue**. Pozwala on odłożyć wykonanie zadania do czasu, aż system wróci do kontekstu wątku (czyli poza ISR). Zadania w work queue są wykonywane w kontekście wątku, co eliminuje ograniczenia ISR.

Podstawowe pojęcia:
- `k_work` - obiekt reprezentujacy pojedyncze zadanie, które może być wykonane w ramach kolejki zadań. Każde zadanie ma przypisaną funkcję (handler), która zostanie wykonana po zleceniu pracy.
- `system workqueue` - Wbudowana w Zephyr RTOS kolejka zadań, która działa w kontekście wątku systemowego. Zadania w tej kolejce są wykonywane w kolejności ich zgłoszenia.
- `k_work_submit()` - funkcja do dodania zadania (`k_work`) do kolejki zadań. Zadanie zostanie wykonane, gdy system na to pozwoli (wątek nie będzie zajęty innymi operacjami).

Użycie w kodzie:
```C++
struct k_work my_work;

void work_handler(struct k_work *work) {
    printk("Praca w work queue: Wykonuję zadanie!\n");
}

static void irq_handler(...) {
    printk("IRQ pin triggered!\n");

    k_work_submit(&work); // Zgłoś zadanie do wykonania
}

void main(void) {
    k_work_init(&my_work, work_handler);
}
```

## **Ćwiczenie 3**

Zdefiniuj w kodzie obsługę przerwania za pomocą workqueue. Zamiast odczytywać dane z modułu NRF24L01 w ISR, zleć to zadanie do wykonania w workqueue. Sprawdź czy kod działa poprawnie i zapisz wyniki...

1. Przenieś odczytanie danych w całości z irq_handler() do nowej funckji `void process_irq_work(struct k_work *work)`
2. W `irq_handler` zamiast odczytywać dane z modułu NRF24L01 zleć to zadanie do wykonania w workqueue.

---

## **Ćwiczenie 4** - Update biblioteki `NRF24`

Teraz należy działający kod przerwań przenieść do biblioteki `NRF24`...