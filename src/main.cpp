#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// =============================================================================
// CONFIGURATION
// =============================================================================

// WiFi credentials
const char *ssid = "FRITZ!Box 75902";
const char *password = "04562358016988474025";

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

// Web server
WebServer server(80);

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
// WEB SERVER HANDLERS
// =============================================================================

void handleRoot()
{
  String html = "<html><head><meta charset='UTF-8'></head><body><h1>IKEA VINDRIKTNING Sensor</h1>";
  html += "<div style='font-size: 72px; font-weight: bold; text-align: left; color: #333; margin: 30px 0;'>";
  html += String(currentPM25);
  html += "</div>";
  html += "<p><a href='/data'>Get data</a></p>";
  html += "<script>setTimeout(function(){location.reload()}, 5000);</script>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleData()
{
  server.send(200, "text/plain", String(currentPM25));
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

  // Initialize WiFi connection
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

  // Initialize sensor buffer
  clearRxBuf();
  Serial.println("Waiting for sensor ...");
}

// =============================================================================
// MAIN LOOP
// =============================================================================

void loop()
{
  // Handle web server requests
  server.handleClient();

  // Wait for data from IKEA sensor
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