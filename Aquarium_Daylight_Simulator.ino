/*
  Aquarium Natural Daylight Simulator
  -----------------------------------
  NodeMCU ESP8266 + SH1106 OLED (1.3" I2C) + WS2812 LED Strip + Test Button

  - Automatic sunrise/sunset via API (latitude/longitude required)
  - OLED displays local time, sunrise, sunset, current phase
  - Realistic fades and brightness changes for each phase
  - Push button activates test mode (full cycle in short time)
  - All pins, connections, and hardware details are in README.md

  IMPORTANT: Edit your country/city coordinates (latitude, longitude)!
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <time.h>
#include <Wire.h>
#include <U8g2lib.h>

// ----------- OLED DISPLAY -----------
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ----------- LED & BUTTON -----------
#define LED_PIN    D4        // GPIO2
#define LED_COUNT  18
#define BUTTON_PIN D3        // GPIO0 (Button to GND)
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// ----------- WIFI (Default SSID/PASS) -----------
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

// ----------- LOCATION -----------
const float latitude = 41.0;      // <-- CHANGE THIS FOR YOUR LOCATION!
const float longitude = 28.6;

// ----------- SUNRISE / SUNSET TIMES -----------
int sunriseHour = 7, sunriseMin = 0, sunsetHour = 19, sunsetMin = 0;
String phaseName = "N/A";

// ----------- COLORS -----------
uint8_t moonlightColor[3] = {0, 0, 10};
uint8_t sunsetColor[3]    = {255, 80, 30};
uint8_t noonColor[3]      = {255, 200, 110};

void setSolidColor(uint8_t* color, float brightness = 1.0);
void softFadeBetweenBrightness(uint8_t* fromColor, uint8_t* toColor, float startB, float endB, int duration);
void runTestMode();
void updateDisplay(struct tm *timeinfo);

void setup() {
  Wire.begin(D2, D1); // I2C OLED
  u8g2.begin();

  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  strip.begin();
  for (int i = 0; i < LED_COUNT; i++) strip.setPixelColor(i, 0, 0, 0);
  strip.show();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(5,32,"WiFi Connecting...");
  u8g2.sendBuffer();

  // WiFi connect
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(5,32,"WiFi Connecting...");
    u8g2.sendBuffer();
  }
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(5,32,"WiFi Connected!");
  u8g2.sendBuffer();
  delay(700);

  // NTP Time
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // UTC+3
  getSunTimes();
}

void loop() {
  static bool testActive = false;
  static unsigned long lastButtonCheck = 0;
  static unsigned long lastScreenUpdate = 0;
  struct tm timeinfo;

  // Test Mode
  if (millis() - lastButtonCheck > 200) {
    lastButtonCheck = millis();
    if (digitalRead(BUTTON_PIN) == LOW && !testActive) {
      testActive = true;
      runTestMode();
      testActive = false;
    }
  }

  if (!getLocalTime(&timeinfo)) {
    setSolidColor(moonlightColor, 0.5);
    phaseName = "No RTC";
    updateDisplay(NULL);
    return;
  }

  int nowHour = timeinfo.tm_hour;
  int nowMin = timeinfo.tm_min;
  float nowFloat = nowHour + nowMin / 60.0;
  float sunrise = sunriseHour + sunriseMin / 60.0;
  float sunset  = sunsetHour + sunsetMin / 60.0;
  float sunriseEnd = sunrise + 0.75;     // 45min
  float sunsetStart = sunset - 0.75;

  // ---- Phase & Fade Management ----
  if (!testActive) {
    if (nowFloat < sunrise) {
      setSolidColor(moonlightColor, 0.1); // Night: 10%
      phaseName = "Moonlight";
    } else if (nowFloat >= sunrise && nowFloat < sunriseEnd) {
      // Sunrise: fade 10% -> 100%, color transition
      softFadeBetweenBrightness(moonlightColor, sunsetColor, 0.1, 1.0, 2000);
      setSolidColor(noonColor, 1.0);
      phaseName = "Sunrise";
    } else if (nowFloat >= sunriseEnd && nowFloat < sunsetStart) {
      setSolidColor(noonColor, 1.0);
      phaseName = "Noon";
    } else if (nowFloat >= sunsetStart && nowFloat < sunset) {
      // Sunset: fade 100% -> 10%, color transition
      softFadeBetweenBrightness(noonColor, sunsetColor, 1.0, 1.0, 1000);
      softFadeBetweenBrightness(sunsetColor, moonlightColor, 1.0, 0.1, 2000);
      setSolidColor(moonlightColor, 0.1);
      phaseName = "Sunset";
    } else {
      setSolidColor(moonlightColor, 0.1);
      phaseName = "Moonlight";
    }
  }

  // ---- OLED Update ----
  if (millis() - lastScreenUpdate > 900) {
    lastScreenUpdate = millis();
    updateDisplay(&timeinfo);
  }
}

// Get sunrise/sunset times from API
void getSunTimes() {
  if (WiFi.status() != WL_CONNECTED) return;

  String url = String("http://api.sunrise-sunset.org/json?lat=") + latitude + "&lng=" + longitude + "&formatted=0";
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);

  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    StaticJsonDocument<1024> doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
      String sunriseStr = doc["results"]["sunrise"];
      String sunsetStr  = doc["results"]["sunset"];

      sunriseHour = sunriseStr.substring(11, 13).toInt() + 3; // UTC+3
      sunriseMin  = sunriseStr.substring(14, 16).toInt();
      sunsetHour  = sunsetStr.substring(11, 13).toInt() + 3; // UTC+3
      sunsetMin   = sunsetStr.substring(14, 16).toInt();

      Serial.printf("Sunrise: %02d:%02d  Sunset: %02d:%02d\n", sunriseHour, sunriseMin, sunsetHour, sunsetMin);
    }
  }
  http.end();
}

void setSolidColor(uint8_t* color, float brightness) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i,
      (uint8_t)(color[0] * brightness),
      (uint8_t)(color[1] * brightness),
      (uint8_t)(color[2] * brightness));
  }
  strip.show();
}

void softFadeBetweenBrightness(uint8_t* fromColor, uint8_t* toColor, float startB, float endB, int duration) {
  const int steps = 80;
  for (int i = 0; i <= steps; i++) {
    float t = (float)i / steps;
    float smooth = t * t * (3 - 2 * t);
    float bright = startB + smooth * (endB - startB);
    uint8_t r = fromColor[0] + smooth * (toColor[0] - fromColor[0]);
    uint8_t g = fromColor[1] + smooth * (toColor[1] - fromColor[1]);
    uint8_t b = fromColor[2] + smooth * (toColor[2] - fromColor[2]);
    for (int j = 0; j < LED_COUNT; j++) {
      strip.setPixelColor(j, r * bright, g * bright, b * bright);
    }
    strip.show();
    delay(duration / steps);
  }
}

void runTestMode() {
  softFadeBetweenBrightness(moonlightColor, sunsetColor, 0.1, 1.0, 1500);
  delay(2000);
  softFadeBetweenBrightness(sunsetColor, noonColor, 1.0, 1.0, 1000);
  delay(2000);
  softFadeBetweenBrightness(noonColor, sunsetColor, 1.0, 1.0, 1000);
  delay(2000);
  softFadeBetweenBrightness(sunsetColor, moonlightColor, 1.0, 0.1, 1500);
  delay(2000);
}

void updateDisplay(struct tm *timeinfo) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  if (timeinfo) {
    char buf[16];
    sprintf(buf, "Time: %02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
    u8g2.drawStr(0, 13, buf);
  } else {
    u8g2.drawStr(0, 13, "Time: ???");
  }
  char buf2[24];
  sprintf(buf2, "Sunrise: %02d:%02d", sunriseHour, sunriseMin);
  u8g2.drawStr(0, 28, buf2);
  sprintf(buf2, "Sunset : %02d:%02d", sunsetHour, sunsetMin);
  u8g2.drawStr(0, 43, buf2);
  u8g2.drawStr(0, 58, ("Phase: " + phaseName).c_str());
  u8g2.sendBuffer();
}
