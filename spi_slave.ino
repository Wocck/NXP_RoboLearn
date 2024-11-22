#include <SPI.h>
#include "driver/spi_slave.h"

// Define SPI Pins
#define PIN_SCK 14  // Clock pin (SCK)
#define PIN_MISO 12 // Master In Slave Out
#define PIN_MOSI 13 // Master Out Slave In
#define PIN_CS 15   // Chip Select (CS)

// Buffers for SPI communication
uint8_t txBuffer[4] = {0x11, 0x22, 0x33, 0x44}; // Data to send to master
uint8_t rxBuffer[4]; // Buffer to store received data

void setup() {
  Serial.begin(115200);

  // SPI Slave configuration
  spi_slave_interface_config_t slave_config = {};
  slave_config.mode = 3;                        // SPI Mode 0 (CPOL=0, CPHA=0)
  slave_config.spics_io_num = PIN_CS;           // CS pin
  slave_config.queue_size = 3;                  // Transaction queue size
  slave_config.flags = 0;                       // No special flags
  slave_config.post_setup_cb = NULL;            // Callback after setup
  slave_config.post_trans_cb = NULL;            // Callback after transaction

  // SPI bus configuration
  spi_bus_config_t bus_config = {};
  bus_config.mosi_io_num = PIN_MOSI;
  bus_config.miso_io_num = PIN_MISO;
  bus_config.sclk_io_num = PIN_SCK;
  bus_config.quadwp_io_num = -1; // Not used
  bus_config.quadhd_io_num = -1; // Not used

  // Initialize the SPI bus as a slave
  esp_err_t ret = spi_slave_initialize(HSPI_HOST, &bus_config, &slave_config, 1);
  if (ret != ESP_OK) {
    Serial.println("Failed to initialize SPI slave!");
    return;
  }

  Serial.println("SPI Slave Initialized");
}

void loop() {
  spi_slave_transaction_t transaction = {};
  transaction.length = 8 * sizeof(rxBuffer); // Transaction length in bits
  transaction.tx_buffer = txBuffer;         // Data to send
  transaction.rx_buffer = rxBuffer;         // Buffer to receive data

  // Wait for a transaction from the master
  esp_err_t ret = spi_slave_transmit(HSPI_HOST, &transaction, portMAX_DELAY);
  if (ret == ESP_OK) {
    Serial.print("Received data: ");
    for (int i = 0; i < sizeof(rxBuffer); i++) {
      Serial.print("0x");
      Serial.print(rxBuffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  } else {
    Serial.println("SPI transaction failed!");
  }
}
