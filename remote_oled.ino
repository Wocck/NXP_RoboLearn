#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_SH110X.h>

// Definicje pinów
#define VRX_PIN 4   // Oś X joysticka (GPIO_4 - ADC2_0)
#define VRY_PIN 15   // Oś Y joysticka (GPIO_2 - ADC2_2)
#define SW_PIN 32   // Przycisk joysticka (GPIO_16)
#define CE_PIN 33   // CE modułu nRF24L01 (GPIO_22)
#define CSN_PIN 5   // CSN modułu nRF24L01 (GPIO_5)
#define LED 2
#define OLED_RESET -1 // Reset OLED (-1 jeśli niewymagany)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

RF24 radio(CE_PIN, CSN_PIN, 100000);
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


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
  delay(2000);

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


  // Mixed X and Y so that orientation of remote is proper (up and down)
  data.joystickX = valueY * ((percentageY >= 0) ? 1 : -1);
  data.joystickY = valueX * ((percentageX >= 0) ? 1 : -1);

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
  radio.setAutoAck(true);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
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
    if (success) {
        if (radio.isAckPayloadAvailable()) {
            char response[32] = {0};
            radio.read(&response, sizeof(response));
            Serial.print("Otrzymano odpowiedź: ");
            Serial.println(response);
        }
    } else {
      Serial.println("Błąd wysyłania danych.");
    }
}

void initializeOLED() {
    if (!display.begin(0x3C, true)) { // Adres I2C wyświetlacza OLED (zazwyczaj 0x3C lub 0x3D)
        Serial.println("Nie można zainicjalizować wyświetlacza OLED!");
        while (1);
    }

    display.clearDisplay();
    display.setTextSize(1);       // Ustaw rozmiar czcionki
    display.setTextColor(SH110X_WHITE); // Ustaw kolor tekstu
    display.display();
    delay(1000);
}

void displayMessage(const char *message) {
    display.clearDisplay();          // Wyczyść ekran
    display.setCursor(0, 0);         // Ustaw początkową pozycję kursora
    display.println("Otrzymano:");   // Stały tekst
    display.println(message);        // Wyświetl wiadomość
    display.display();               // Aktualizuj ekran
}

void setup() {
  Serial.begin(115200);
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(LED, OUTPUT);

  calibrateJoystick();
  initializeRF24();
  initializeOLED();
}

void loop() {
  readJoystick();
  sendData();
  delay(50);
}
