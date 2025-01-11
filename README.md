# Zdalnie sterwoany robot


## Wprowadzenie

W ramach tego ćwiczenia zbudujemy aplikację dla zdalnie sterowanego robota, który wykorzystuje moduł radiowy nRF24L01+ do odbierania danych sterujących oraz silniki DC do ruchu. Będziemy korzystać z systemu Zephyr RTOS, a aplikacja zostanie podzielona na dwa etapy:
1. **Prosta pętla sterująca:** Robot sterowany w pętli głównej.
2. **Sterowanie wielowątkowe:** Oddzielenie logiki sterowania modułem radiowym i silnikami do osobnych wątków.

## Cele

- Zrozumienie integracji modułów komunikacyjnych (nRF24L01+) z mikrokontrolerem.
- Poznanie sposobu sterowania silnikami DC za pomocą PWM i GPIO.
- Nauka podziału zadań na wątki w Zephyr RTOS.

---

## Wymagane komponenty

- Mikrokontroler NXP MIMXRT1064-EVK.
- Moduł radiowy nRF24L01+.
- Silniki DC z mostkiem H (np. L298N).
- Połączenie GPIO, PWM i SPI.

---

## Etap 1: Prosta pętla sterująca

### Zadanie

Twoim celem jest zbudowanie aplikacji, która w pętli głównej:
1. Odczytuje dane z modułu nRF24L01+ (dane joysticka i przycisków).
2. Steruje silnikami na podstawie otrzymanych danych.

### Instrukcja

1. **Inicjalizacja urządzeń:**
   - Użyj `DEVICE_DT_GET` do pobrania urządzeń SPI i GPIO.
   - Zainicjalizuj klasy `NRF24` oraz `Engine` za pomocą odpowiednich urządzeń.

2. **Implementacja pętli głównej:**
   - Sprawdź, czy moduł radiowy odbiera dane.
   - Odbierz dane pakietu joysticka za pomocą metody `radio.get_current_packet()`.
   - Wypisz dane w konsoli za pomocą `printk`.
   - Wywołaj `engine.controlMotors(packet)` w celu sterowania silnikami.

3. **Kod aplikacji:**
   Zaimplementuj logikę w pliku `src/main.cpp`.

---

## Etap 2: Sterowanie wielowątkowe

### Zadanie

Twoim celem jest rozdzielenie logiki sterowania radiowego i silnikami na dwa oddzielne wątki. 

### Instrukcja

1. **Tworzenie wątków:**
   - Skonfiguruj dwa wątki: jeden dla obsługi modułu nRF24, drugi dla silników.
   - Każdy wątek powinien korzystać z `k_thread_create`.

2. **Wątek radiowy:**
   - Odbieraj pakiety danych z nRF24 w sposób ciągły.
   - Buforuj otrzymane dane w zmiennej globalnej chronionej mutexem.

3. **Wątek silnikowy:**
   - Pobieraj dane z bufora i steruj silnikami na podstawie najnowszych danych.

4. **Synchronizacja:**
   - Użyj mechanizmów synchronizacji Zephyr, takich jak mutex (`k_mutex`) lub semafory (`k_sem`), aby zapewnić bezpieczny dostęp do danych.

5. **Kod aplikacji:**
   - Zaimplementuj wątki w plikach `src/main.cpp` oraz `src/uart_bridge.cpp`.


## Zadania

1. **Etap 1:**
   - W pliku `src/engine.cpp` zaimplementuj funkcję która zmapuje dane z Joysticka na sygnały PWM dla silników.
   - W pliku `src/main.cpp` zaimplementuj pętlę główną odbierającą dane z modułu radiowego i sterującą silnikami.

2. **Etap 2:**
   - W pliku `src/main.cpp` zaimplementuj wątki dla modułu radiowego i silników.
   - Użyj mechanizmów synchronizacji, aby zapewnić bezpieczny dostęp do danych.

3. **Testowanie:**
   - Przetestuj działanie robota w obu etapach.
   - Sprawdź, czy robot reaguje na dane joysticka w czasie rzeczywistym.

## Podpowiedzi

1. **NRF24L01+:**
   - Moduł radiowy odbiera pakiety danych w strukturze `DataPacket`. Użyj metody `radio.get_current_packet()` do ich odczytu.

2. **Sterowanie silnikami:**
   - Klasa `Engine` obsługuje kontrolę prędkości i kierunku silników za pomocą metod GPIO i PWM.
   - Wywołaj metodę `engine.controlMotors(packet)`, przekazując dane joysticka.

3. **Wielowątkowość w Zephyr:**
   - Użyj `k_thread_create` do utworzenia wątków.
   - Skonfiguruj stos i priorytety wątków, np.:
     ```C++
     K_THREAD_STACK_DEFINE(radio_stack, STACK_SIZE);
     struct k_thread radio_thread_data;
     ```

4. **Synchronizacja:**
   - Użyj `k_mutex` do ochrony zmiennych globalnych podczas zapisu i odczytu.

