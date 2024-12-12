#include <nRF24L01.h>
#include <RF24.h>

// Pin definitions
#define CE_PIN 33   // CE pin for nRF24L01
#define CSN_PIN 5   // CSN pin for nRF24L01
#define LED 2       // LED pin for visual feedback

RF24 radio(CE_PIN, CSN_PIN, 100000);

const byte address[5] = {0xCC, 0xCE, 0xCC, 0xCE, 0xCC};
uint8_t receivedData[4]; // Buffer to hold received data

void initializeRF24() {
  if (!radio.begin()) {
    Serial.println("nRF24L01 module not detected. Check connections!");
    while (1);
  }
  radio.setAutoAck(false);
  radio.setAddressWidth(5);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(76);
  radio.setPayloadSize(4);
  radio.openReadingPipe(0, address);

  radio.startListening(); 
  
  Serial.println("nRF24L01 module initialized and listening.");
  radio.printDetails();
}

void receiveData() {
  if (radio.available()) {
    // Read 4-byte payload into the receivedData buffer
    radio.read(receivedData, sizeof(receivedData));

    // Print received data for debugging
    Serial.print("Received Data: ");
    for (int i = 0; i < sizeof(receivedData); i++) {
      Serial.print((char)receivedData[i]); // Print each byte as a character
      Serial.print(" ");
    }
    Serial.println();

    // Visual feedback: Turn on LED if data matches 'HI' pattern
    if (receivedData[0] == 'H' && receivedData[1] == 'I') {
      digitalWrite(LED, HIGH); // Turn LED on for 'HI'
    } else {
      digitalWrite(LED, LOW); // Turn LED off otherwise
    }
  } else {
    Serial.println("No data received.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  // Initialize the nRF24L01 module
  initializeRF24();
}

void loop() {
  // Continuously check for received data
  receiveData();
  //radio.printDetails();

  delay(1000); // Small delay for smoother operation
}
