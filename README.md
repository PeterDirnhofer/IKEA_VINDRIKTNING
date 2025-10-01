# IKEA VINDRIKTNING ESP32 Reader

ESP32 reads IKEA Feinstaubsensor VINDRIKTNING via UART2 serial interface and serves the data over HTTP web server.

## Features

- Reads PM2.5 data from IKEA VINDRIKTNING sensor
- Serial Monitor output at 115200 Baud
- HTTP web server for remote data access
- Real-time web interface with auto-refresh

## Hardware Setup

### Communication Details
- **Interface**: UART2 (ESP32 to IKEA sensor)
- **Baud rate**: 9600
- **Voltage**: 3.3V on ESP32 side, 5V on IKEA sensor side
  - ⚠️ **Important**: Use a voltage divider to protect the ESP32 input!
- **GPIO16**: RX2 (receiving data from sensor)
- **GPIO17**: TX2 (not used; no data sent to IKEA sensor)

### Wiring
Connect the IKEA sensor's data line to ESP32 GPIO16 through a voltage divider circuit.

## Usage

### Web Interface
- Access sensor data: `http://[ESP32_IP]/`
- Get raw data: `http://[ESP32_IP]/data`
- Example: `curl http://192.168.178.51/data`

### Serial Monitor
Monitor real-time PM2.5 values at 115200 baud rate.

## Configuration

Update WiFi credentials in `src/main.cpp`:
```cpp
const char *ssid = "YOUR_WIFI_SSID";
const char *password = "YOUR_WIFI_PASSWORD";
```

## Data Format

The sensor sends 20-byte packets with the following header validation:
- Bytes 0-4: `0x16 0x11 0x0B 0x00 0x00`
- Bytes 5-6: PM2.5 value (16-bit, big-endian)

## References

- Based on: [esp8266-vindriktning-particle-sensor](https://github.com/Hypfer/esp8266-vindriktning-particle-sensor)
- ESP32 UART2 tutorial: [YouTube Guide](https://youtu.be/GwShqW39jlE)
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
