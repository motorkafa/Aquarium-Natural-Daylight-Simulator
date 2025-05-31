# Aquarium-Natural-Daylight-Simulator
ESP8266-based aquarium light system with WS2812 LEDs and OLED. Automatically simulates sunrise, sunset, and moonlight using real solar times for your location
# Aquarium Natural Daylight Simulator (ESP8266 + SH1106 OLED + WS2812)

This project simulates a realistic sunrise–noon–sunset–moonlight cycle for aquarium lighting using an ESP8266 (NodeMCU), a SH1106 (1.3" I2C OLED display), a WS2812 RGB LED strip (18 LEDs), and a push-button for test mode. It automatically calculates local sunrise and sunset times based on your latitude and longitude by fetching data from an online API, and displays the current phase, local time, sunrise, and sunset on the OLED screen.

## Features

- **Automatic sunrise/sunset calculation** via internet (based on your location).
- **Realistic fade transitions** between moonlight, sunrise, noon, sunset phases.
- **Animated test mode** via push-button.
- **OLED status display**: shows current time, sunrise & sunset times, and current lighting phase.
- **All settings are fully automated after powering up.**
- **Fully open-source and customizable.**

---

## **Required Hardware**

| Part                | Model/Type          | Notes                    |
|---------------------|---------------------|--------------------------|
| Microcontroller     | NodeMCU ESP8266     | (ESP-12E recommended)    |
| OLED Display        | 1.3" SH1106 I2C     | 128x64 px, I2C, 0x3C     |
| LED Strip           | WS2812 (NeoPixel)   | 18 LEDs, 5V              |
| Push Button         | Standard Tactile    | Test mode activation     |
| Power Supply        | 5V/2A (min)         | For LED strip            |
| Resistor            | 330–470Ω            | Between ESP DATA pin and LED DIN |
| Capacitor           | 1000 µF 16V         | between the +5V and GND lines of the WS2812 LED strip |

---

## **Connections**

### **NodeMCU Pinout (ESP8266)**
| Device     | Function   | NodeMCU Pin | ESP GPIO | Note         |
|------------|------------|-------------|----------|--------------|
| OLED SDA   | I2C Data   | D2          | GPIO4    |              |
| OLED SCL   | I2C Clock  | D1          | GPIO5    |              |
| OLED VCC   | Power      | 3V3         | —        | 3.3V supply on esp8266 3v pin |
| OLED GND   | Ground     | GND         | —        |              |
| WS2812 DIN | Data In    | D4          | GPIO2    | 330Ω resistor recommended |
| WS2812 VCC | Power      | VIN/VU      | —        | 5V           |
| WS2812 GND | Ground     | GND         | —        |              |
| Button     | One leg    | D3          | GPIO0    |              |
| Button     | Other leg  | GND         | —        |              |

**Note:**  
- Use a common ground between the ESP8266 and the LED strip.
- If you power the LED strip with an external adapter, always connect its GND to NodeMCU GND.
- A 1000 µF 16V electrolytic capacitor is added between the +5V and GND lines of the WS2812 LED strip (as close to the strip's power input as possible) to help stabilize voltage and prevent power fluctuations.

---

## **How It Works**

- On power up, the system connects to WiFi.
- Fetches sunrise/sunset times for your location.
- Animates the WS2812 LED strip through a natural day–night cycle.
- OLED shows current time, sunrise, sunset, and phase.
- Test button instantly triggers a full cycle animation for demo/testing.

---

## **IMPORTANT:**
**Be sure to edit the latitude and longitude in the code (`const float latitude`, `const float longitude`) for your country/city!  
Default is Istanbul, Turkey 41.0, 28.6**
**In the code, you need to change the 18 LED part as much as your own number of LEDs, otherwise it may light up less or incorrectly.

---

## **Libraries Used**

- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)
- [U8g2 by olikraus](https://github.com/olikraus/U8g2_Arduino)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
- [ESP8266WiFi](https://github.com/esp8266/Arduino)
- [ESP8266HTTPClient](https://github.com/esp8266/Arduino)

---

## **Example Images**
*(Insert your wiring photo or working project images here)*

---

