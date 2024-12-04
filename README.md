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

