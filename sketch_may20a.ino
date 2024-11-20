#include <nRF24L01.h>
#include <RF24.h>

// Definicje pinów
#define VRX_PIN 4  // Oś X joysticka (GPIO_4 - ADC2_0)
#define VRY_PIN 2  // Oś Y joysticka (GPIO_2 - ADC2_2)
#define SW_PIN 16  // Przycisk joysticka (GPIO_16)
#define CE_PIN 22  // CE modułu nRF24L01 (GPIO_22)
#define CSN_PIN 5  // CSN modułu nRF24L01 (GPIO_5)

// Tworzenie instancji modułu nRF24L01
RF24 radio(CE_PIN, CSN_PIN);

// Struktura danych do wysyłania
struct DataPacket {
  int joystickX;       // Wartość osi X joysticka
  int joystickY;       // Wartość osi Y joysticka
  bool buttonPressed;  // Stan przycisku joysticka
};

DataPacket data;

// Zmienna zakresów kalibracji
int minX, maxX, minY, maxY;
const byte address[6] = "00001";

// Funkcja kalibracji joysticka
void calibrateJoystick() {
  int centerX = analogRead(VRX_PIN);
  int centerY = analogRead(VRY_PIN);

  // Ustawienie zakresu kalibracji (z tolerancją)
  minX = centerX - 200;
  maxX = centerX + 200;
  minY = centerY - 200;
  maxY = centerY + 200;

  Serial.print("Kalibracja X: ");
  Serial.print(minX);
  Serial.print(" - ");
  Serial.println(maxX);

  Serial.print("Kalibracja Y: ");
  Serial.print(minY);
  Serial.print(" - ");
  Serial.println(maxY);
}

// Funkcja odczytu wartości joysticka
void readJoystick() {
  int rawX = analogRead(VRX_PIN);
  int rawY = analogRead(VRY_PIN);

  // Normalizacja do zakresu -100 do 100
  data.joystickX = map(rawX, minX, maxX, -100, 100);
  data.joystickY = map(rawY, minY, maxY, -100, 100);

  // Odczyt przycisku
  data.buttonPressed = !digitalRead(SW_PIN);
}

// Funkcja inicjalizacji modułu nRF24L01
void initializeRF24() {
  if (!radio.begin()) {
    Serial.println("Moduł nRF24L01 nie został wykryty. Sprawdź połączenia!");
    while (1);
  }

  // Konfiguracja modułu radiowego
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.stopListening();

  Serial.println("Moduł nRF24L01 gotowy.");
  radio.printDetails();
}

// Funkcja wysyłania danych
void sendData() {
  bool success = radio.write(&data, sizeof(data));
  if (success) {
    Serial.println("Dane wysłane.");
  } else {
    Serial.println("Błąd wysyłania danych.");
  }
}

// Inicjalizacja
void setup() {
  Serial.begin(115200);
  pinMode(SW_PIN, INPUT_PULLUP);  // Konfiguracja przycisku joysticka

  // Kalibracja joysticka
  calibrateJoystick();

  // Inicjalizacja modułu nRF24L01
  initializeRF24();
}

// Główna pętla programu
void loop() {
  // Odczyt joysticka
  readJoystick();

  // Debugowanie wartości joysticka
  Serial.print("X: ");
  Serial.print(data.joystickX);
  Serial.print(" | Y: ");
  Serial.print(data.joystickY);
  Serial.print(" | Przyciski: ");
  Serial.println(data.buttonPressed);

  // Wysyłanie danych
  sendData();

  delay(1000);  // Opóźnienie dla stabilności
}
