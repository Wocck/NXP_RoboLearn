#include <Arduino.h>

// Definicje pinów dla zewnętrznego UART
#define UART_RX_PIN 3 // RX pin dla zewnętrznego UART (GPIO3)
#define UART_TX_PIN 1 // TX pin dla zewnętrznego UART (GPIO1)

// Definicje parametrów UART
#define UART_BAUD_RATE 115200         // Prędkość transmisji
#define UART_CONFIG SERIAL_8N1        // Format ramki (8 bitów danych, brak parzystości, 1 bit stopu)
                                      // N (None): brak parzystości (domyślnie).
                                      // E (Even): parzystość.
                                      // O (Odd): nieparzystość.
#define UART_TIMEOUT 1000             // Timeout w milisekundach
#define UART_BUFFER_SIZE 256          // Rozmiar bufora UART

void setup() {
  // Inicjalizacja portu szeregowego USB
  Serial.begin(UART_BAUD_RATE);
  while (!Serial) {
    ; // Czekaj na otwarcie portu szeregowego
  }
  Serial.println("UART Bridge Started");

  // Inicjalizacja zewnętrznego UART-a
  Serial1.begin(UART_BAUD_RATE, UART_CONFIG, UART_RX_PIN, UART_TX_PIN);
  Serial1.setTimeout(UART_TIMEOUT); // Ustawienie timeoutu dla odczytu UART
}

void loop() {
  // Przekazywanie danych z USB (Serial) do zewnętrznego UART (Serial1)
  while (Serial.available()) {
    char incomingByte = Serial.read();
    Serial1.write(incomingByte);
  }

  // Przekazywanie danych z zewnętrznego UART (Serial1) do USB (Serial)
  while (Serial1.available()) {
    char incomingByte = Serial1.read();
    Serial.write(incomingByte);
  }

  // Opcjonalne opóźnienie dla zmniejszenia obciążenia CPU
  delay(10);
}
