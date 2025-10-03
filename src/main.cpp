#include <Arduino.h>
#include "BluetoothSerial.h"

// =============================================================================
// CONFIGURATION
// =============================================================================

// Define pins for RX and TX
#define RXD2 16 // GPIO16 as RX
#define TXD2 17 // GPIO17 as TX (not used but must be defined)

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

// UART2 communication with IKEA sensor
HardwareSerial ikeaSerial(2);

// Buffer for received data
uint8_t serialRxBuf[20];
uint8_t rxBufIdx = 0;

// Bluetooth Serial
BluetoothSerial SerialBT;

// Current PM2.5 value
int currentPM25 = 0;

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

void clearRxBuf()
{
  memset(serialRxBuf, 0, sizeof(serialRxBuf));
  rxBufIdx = 0;
}

// =============================================================================
// BLUETOOTH HANDLERS
// =============================================================================

void handleBluetoothRequests()
{
  if (SerialBT.available())
  {
    String command = SerialBT.readString();
    command.trim();

    if (command == "data" || command == "DATA")
    {
      SerialBT.println(currentPM25);
    }
    else if (command == "status" || command == "STATUS")
    {
      SerialBT.println("IKEA VINDRIKTNING Sensor");
      SerialBT.print("Current PM2.5: ");
      SerialBT.println(currentPM25);
    }
    else
    {
      SerialBT.println("Available commands: data, status");
    }
  }
}

// =============================================================================
// SETUP FUNCTION
// =============================================================================

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(500); // Give time to switch USB from programming to Serial Monitor

  // Initialize UART2 communication with IKEA Vindriktning
  ikeaSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);

  if (!ikeaSerial)
  {
    Serial.println("Invalid Serial configuration. Check RX/TX pins.");
  }
  else
  {
    Serial.println("+++ UART2 to IKEA sensor initialized");
  }

  // Initialize Bluetooth
  if (!SerialBT.begin("BT_001"))
  {
    Serial.println("An error occurred initializing Bluetooth");
  }
  else
  {
    Serial.println("+++ Bluetooth initialized. Device name: BT_001");
    Serial.println("Ready to pair!");
  }

  // Initialize sensor buffer
  clearRxBuf();
  Serial.println("Waiting for sensor ...");
}

// =============================================================================
// MAIN LOOP
// =============================================================================

void loop()
{
  // Handle Bluetooth requests
  handleBluetoothRequests();

  // Wait for data from IKEA sensor
  while (!ikeaSerial.available())
  {
    handleBluetoothRequests(); // Continue handling Bluetooth requests while waiting
    delay(1);
  }

  // Read 20-byte data packet from sensor
  while (ikeaSerial.available())
  {
    serialRxBuf[rxBufIdx++] = ikeaSerial.read();
    delay(15); // Prevent data corruption

    if (rxBufIdx > 20)
    {
      clearRxBuf(); // Avoid buffer overflow
    }
  }

  // Validate data packet header (first 5 bytes must match)
  bool headerValid = (serialRxBuf[0] == 0x16) &&
                     (serialRxBuf[1] == 0x11) &&
                     (serialRxBuf[2] == 0x0B) &&
                     (serialRxBuf[3] == 0x00) &&
                     (serialRxBuf[4] == 0x00);

  if (headerValid && rxBufIdx == 20)
  {
    // Extract PM2.5 value from bytes 5 and 6
    currentPM25 = (serialRxBuf[5] << 8) | serialRxBuf[6];

    // Output value to Serial Monitor
    Serial.println(currentPM25);

    // Send data via Bluetooth automatically
    SerialBT.print("#");
    SerialBT.println(currentPM25);

    // Debug: print full raw data (uncomment if needed)
    /*
    for (int i = 0; i < rxBufIdx; i++) {
      Serial.printf("%02x ", serialRxBuf[i]);
    }
    Serial.println();
    */
  }

  clearRxBuf();
}