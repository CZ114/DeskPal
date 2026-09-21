# Hardware and setup

## Hardware

- Seeed Wio Terminal with its onboard LIS3DHTR accelerometer, light sensor, LCD and top buttons.
- External Sensirion SHT40 temperature/humidity sensor on the external I2C bus (`Wire`), address `0x44`.
- USB data cable and suitable host computer.
- Wi-Fi network and an MQTT broker on a trusted LAN reachable by both device and client.

The accelerometer is accessed through `Wire1`. Use the Seeed Wio Terminal documentation for the correct Grove/I2C wiring. Verify voltage and connector pinout before connection.

## Arduino dependencies

Select **Seeeduino Wio Terminal** using the Seeed SAMD board package. Install the libraries supplying these headers:

| Header | Dependency |
| --- | --- |
| `PoseDetection_inferencing.h` | Recovered library included in this repository |
| `LIS3DHTR.h` | Seeed LIS3DHTR accelerometer library |
| `TFT_eSPI.h` | Wio Terminal-compatible Seeed TFT support |
| `rpcWiFi.h`, `WiFi.h`, `WiFiUdp.h` | Seeed Wio Terminal Wi-Fi support and board dependencies |
| `PubSubClient.h` | PubSubClient |
| `ArduinoJson.h` | ArduinoJson; source uses the v6-style `StaticJsonDocument` API |
| `SensirionI2cSht4x.h` | Sensirion I2C SHT4x and Sensirion Core dependencies |
| `NTPClient.h` | NTPClient |
| `Wire.h`, `Arduino.h` | Board core |

Original versions of the board package and external dependencies were not found. This is a recovery recipe, not a verified lockfile. Do not replace Wio-specific display/network support with an arbitrary similarly named library.

Copy `libraries/PoseDetection_inferencing` into the Arduino sketchbook `libraries` directory. The export includes generic Edge Impulse examples for other boards; those are not DeskPal entry points. Its `library.properties` also lists generic example dependencies, which are not evidence that DeskPal requires those boards.

## Private configuration

Copy `firmware/DeskPal/config.example.h` to `config.h` in the same folder. Replace all example values. Keep `config.h` out of Git. `192.0.2.1` is only a documentation placeholder and will not connect to your broker.

The preserved firmware uses unauthenticated MQTT over TCP port 1883 and blocking reconnect loops. Use a controlled LAN. TLS and MQTT authentication proposed in the coursework are not implemented here.

Open `firmware/DeskPal/DeskPal.ino`, choose the Wio Terminal board and serial port, then compile and upload. Open Serial Monitor at **115200 baud**. Follow [DEMO.md](DEMO.md).

## Verification status

Repository structure, credential removal, source provenance, ZIP integrity and model metadata can be validated without hardware using `python scripts/validate_repository.py`. This does not compile the Arduino program or validate its timing, sensors, network behavior or classifier performance.

See [KNOWN_LIMITATIONS.md](KNOWN_LIMITATIONS.md) for original-code issues that remain intentionally documented rather than silently changed.
