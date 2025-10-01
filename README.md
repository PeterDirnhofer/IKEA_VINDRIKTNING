# IKEA VINDRIKTNING ESP32 Web Server

This project reads PM2.5 data from an IKEA VINDRIKTNING air quality sensor using an ESP32 microcontroller and serves the data via a web server.

## Features

- 📊 Real-time PM2.5 air quality monitoring
- 🌐 Web interface with auto-refresh every 5 seconds
- 📡 HTTP API endpoint for data access
- 🔧 Simple curl command support for automation

## Hardware Requirements

- ESP32 development board
- IKEA VINDRIKTNING air quality sensor
- Voltage divider circuit (to step down 5V to 3.3V)
- Jumper wires

## Wiring

| ESP32 Pin | IKEA Sensor | Description |
|-----------|-------------|-------------|
| GPIO16    | TX (Pin 2)  | UART2 RX    |
| GND       | GND (Pin 1) | Ground      |
| 3.3V      | VCC (Pin 3) | Power       |

**Important:** Use a voltage divider to protect the ESP32 input as the IKEA sensor outputs 5V signals.

## Software Setup

1. Install Arduino IDE with ESP32 board support
2. Install required libraries:
   - WiFi (built-in)
   - WebServer (built-in)

3. Update WiFi credentials in `main.cpp`:
   ```cpp
   const char *ssid = "YOUR_WIFI_SSID";
   const char *password = "YOUR_WIFI_PASSWORD";
   ```

4. Upload the code to your ESP32

## Usage

### Web Interface

1. Open the Arduino Serial Monitor (115200 baud)
2. Find the ESP32's IP address in the output
3. Open a web browser and navigate to `http://[ESP32_IP]/`
4. View real-time PM2.5 readings with auto-refresh

### API Access

#### Get PM2.5 Value (Plain Text)
```bash
curl http://[ESP32_IP]/data
```
Returns: `42` (example PM2.5 value in μg/m³)

#### Example Usage in Scripts
```bash
# Save value to variable
PM25_VALUE=$(curl -s http://192.168.1.123/data)
echo "Current PM2.5: $PM25_VALUE μg/m³"

# Save to file
curl -s http://192.168.1.123/data > pm25_value.txt
```

## Technical Details

- **Communication:** UART2 at 9600 baud
- **Data Format:** 20-byte packets with specific header validation
- **Update Rate:** Real-time as data arrives from sensor
- **Web Server:** Port 80 (standard HTTP)


## Troubleshooting

- **No WiFi connection:** Check SSID and password
- **No sensor data:** Verify wiring and voltage divider
- **Can't access web page:** Ensure device is on same network
- **Compilation errors:** Check ESP32 board installation

## Based On

- Original project: [Hypfer/esp8266-vindriktning-particle-sensor](https://github.com/Hypfer/esp8266-vindriktning-particle-sensor)
- More info: [PeterDirnhofer/IKEA-vindriktning-ESP32-Bluetooth](https://github.com/PeterDirnhofer/IKEA-vindriktning-ESP32-Bluetooth)

## License

This project is open source. Feel free to modify and distribute.
