#include <nRF24L01.h>
#include <RF24.h>

// Definicje pinów
#define VRX_PIN 4   // Oś X joysticka (GPIO_4 - ADC2_0)
#define VRY_PIN 15   // Oś Y joysticka (GPIO_2 - ADC2_2)
#define SW_PIN 32   // Przycisk joysticka (GPIO_16)
#define CE_PIN 33   // CE modułu nRF24L01 (GPIO_22)
#define CSN_PIN 5   // CSN modułu nRF24L01 (GPIO_5)
#define LED 2

RF24 radio(CE_PIN, CSN_PIN, 100000);

struct __attribute__((packed)) DataPacket {
  int8_t joystickX;
  int8_t joystickY;
  uint8_t buttonPressed;
};

DataPacket data;

int centerX, centerY;
int maxDeflectionPositiveX, maxDeflectionNegativeX;
int maxDeflectionPositiveY, maxDeflectionNegativeY;

int rawX;
int rawY;
const byte address[5] = {0xCC, 0xCE, 0xCC, 0xCE, 0xCC};

void calibrateJoystick() {
  Serial.println("Kalibracja joysticka. Proszę nie ruszać joysticka.");
  delay(2000); // Czekaj, aby ustabilizować odczyty

  // Odczyt wartości środkowych
  centerX = analogRead(VRX_PIN);
  centerY = analogRead(VRY_PIN);

  // Maksymalne odchylenia (zakładając 12-bitowy ADC od 0 do 4095)
  maxDeflectionPositiveX = 4095 - centerX;
  maxDeflectionNegativeX = centerX - 0;
  maxDeflectionPositiveY = 4095 - centerY;
  maxDeflectionNegativeY = centerY - 0;

  Serial.print("Kalibracja środka X: ");
  Serial.println(centerX);
  Serial.print("Kalibracja środka Y: ");
  Serial.println(centerY);
}

void readJoystick() {
  rawX = analogRead(VRX_PIN);
  rawY = analogRead(VRY_PIN);

  int deltaX = rawX - centerX;
  int deltaY = rawY - centerY;

  float percentageX = 0;
  float percentageY = 0;

  // Obliczanie procentowego odchylenia dla osi X
  if (deltaX >= 0) {
    percentageX = pow((float)deltaX / maxDeflectionPositiveX, 1.5) * 100.0;
  } else {
    percentageX = -pow((float)-deltaX / maxDeflectionNegativeX, 1.5) * 100.0;
  }

  // Obliczanie procentowego odchylenia dla osi Y
  if (deltaY >= 0) {
    percentageY = pow((float)deltaY / maxDeflectionPositiveY, 1.5) * 100.0;
  } else {
    percentageY = -pow((float)-deltaY / maxDeflectionNegativeY, 1.5) * 100.0;
  }

  // Zastosowanie martwej strefy ±5%
  if (percentageX > -5 && percentageX < 5) {
    percentageX = 0;
  }
  if (percentageY > -5 && percentageY < 5) {
    percentageY = 0;
  }

  // Zaokrąglenie do najbliższej wartości dziesiętnej
  int valueX = ((int)(abs(percentageX) / 10)) * 10;
  int valueY = ((int)(abs(percentageY) / 10)) * 10;

  if (valueX > 90) valueX = 90;
  if (valueY > 90) valueY = 90;

  // Przywrócenie znaku odchylenia
  // Mixed X and Y so that orientation of remote is proper (up and down)
  data.joystickX = valueY * ((percentageY >= 0) ? 1 : -1);
  data.joystickY = valueX * ((percentageX >= 0) ? 1 : -1);

  // Odczyt przycisku
  data.buttonPressed = !digitalRead(SW_PIN);

  if(data.buttonPressed != 0 || data.joystickX != 0 || data.joystickY != 0){
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }
}

void initializeRF24() {
  if (!radio.begin()) {
    Serial.println("Moduł nRF24L01 nie został wykryty. Sprawdź połączenia!");
    while (1);
  }

  // Konfiguracja modułu radiowego
  radio.setAutoAck(false);
  radio.setAddressWidth(5);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(76);
  radio.setPayloadSize(3);
  radio.openWritingPipe(address);
  Serial.println("Moduł nRF24L01 gotowy:");
  radio.printDetails();
}

void sendData() {
  bool success = radio.write(&data, sizeof(data));
  Serial.println(sizeof(data));
  if (success) {
    Serial.println("Dane wysłane.");
  } else {
    Serial.println("Błąd wysyłania danych.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(SW_PIN, INPUT_PULLUP);  // Konfiguracja przycisku joysticka
  pinMode(LED, OUTPUT);

  // Kalibracja joysticka
  calibrateJoystick();

  // Inicjalizacja modułu nRF24L01
  initializeRF24();
}

void loop() {
  // Odczyt joysticka
  readJoystick();

  // Debugowanie wartości joysticka
  Serial.print("\r");
  Serial.print("X: ");
  //Serial.print(rawX);
  Serial.print(data.joystickX);
  Serial.print(" | Y: ");
  //Serial.print(rawY);
  Serial.print(data.joystickY);
  Serial.print(" | Przyciski: ");
  Serial.println(data.buttonPressed);

  // Wysyłanie danych
  sendData();

  delay(100);  // Mniejsze opóźnienie dla płynniejszego odczytu
}
