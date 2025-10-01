#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

/*****************************************************************
  Find more information on:
  https://github.com/PeterDirnhofer/IKEA-vindriktning-ESP32-Bluetooth

  Story:
  ESP32 reads IKEA Feinstaubsensor VINDRIKTNING
  via UART2 serial interface.
  Data can be monitored on Arduino's Serial Monitor at 115200 Baud.

  Additionally, data is served over HTTP web server.

  Code is based on:
  https://github.com/Hypfer/esp8266-vindriktning-particle-sensor

  ***********************************************************************
  Details:
  Communication between IKEA sensor and ESP32:
  - ESP32 uses UART2 (class: ikeaSerial)
  - Baud rate: 9600
  - Voltage: 3.3V on ESP32 side, 5V on IKEA sensor side
    -> Use a voltage divider to protect the ESP32 input!
  - GPIO16 = RX2 (receiving data)
  - GPIO17 = TX2 (not used; no data sent to IKEA sensor)

  A nice explanation of how to use UART2 on the ESP32:
  https://youtu.be/GwShqW39jlE

  ***********************************************************************
  Additionally, ESP32 serves measured data via HTTP web server.
  Access sensor data by visiting http://[ESP32_IP]/data in your browser.
***********************************************************************/

// WiFi credentials
const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";

/***********************************************************************************************/
// Define pins for RX and TX
#define RXD2 16 // GPIO16 as RX
#define TXD2 17 // GPIO17 as TX (not used but must be defined)

// UART2 communication with IKEA sensor
HardwareSerial ikeaSerial(2);

// Buffer for received data
uint8_t serialRxBuf[20];
uint8_t rxBufIdx = 0;

// Web server
WebServer server(80);

// Current PM2.5 value
int currentPM25 = 0;

/*******************************************************************/
void clearRxBuf()
{
  memset(serialRxBuf, 0, sizeof(serialRxBuf));
  rxBufIdx = 0;
}

void handleRoot()
{
  String html = "<html><body><h1>IKEA VINDRIKTNING Sensor</h1>";
  html += "<p>Current PM2.5 value: <strong>" + String(currentPM25) + " μg/m³</strong></p>";
  html += "<p><a href='/data'>Get JSON data</a></p>";
  html += "<script>setTimeout(function(){location.reload()}, 5000);</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleData()
{
  String json = "{\"pm25\":" + String(currentPM25) + "}";
  server.send(200, "application/json", json);
}

/*************************** Setup ************************************/
void setup()
{
  Serial.begin(115200);
  delay(500); // Give time to switch USB from programming to Serial Monitor

  // Initialize serial communication with IKEA Vindriktning
  ikeaSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);

  if (!ikeaSerial)
  {
    Serial.println("Invalid Serial configuration. Check RX/TX pins.");
  }
  else
  {
    Serial.println("+++ UART2 to IKEA sensor initialized");
  }

  // Initialize WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("+++ WiFi connected! IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize Web Server
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("+++ Web server started");

  clearRxBuf();
  Serial.println("Waiting for sensor ...");
}

/*************************** Loop ************************************/
void loop()
{
  server.handleClient(); // Handle web server requests

  while (!ikeaSerial.available())
  {
    server.handleClient(); // Continue handling web requests while waiting
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

  // Check header for validity: first 5 bytes must match
  bool headerValid =
      serialRxBuf[0] == 0x16 &&
      serialRxBuf[1] == 0x11 &&
      serialRxBuf[2] == 0x0B &&
      serialRxBuf[3] == 0x00 &&
      serialRxBuf[4] == 0x00;

  if (headerValid && rxBufIdx == 20)
  {
    // Get PM2.5 value (bytes 5 and 6)
    currentPM25 = (serialRxBuf[5] << 8) | serialRxBuf[6];

    // Send value to Serial Monitor
    Serial.println(currentPM25);

    // Debug: print full raw data
    /*
    for (int i = 0; i < rxBufIdx; i++) {
      Serial.printf("%02x ", serialRxBuf[i]);
    }
    Serial.println();
    */
  }

  clearRxBuf();
}