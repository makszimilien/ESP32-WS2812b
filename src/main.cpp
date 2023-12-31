#include "SPIFFS.h"
#include "filehandling.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FastLED.h>
#include <WiFi.h>
#include <esp_task_wdt.h>

// How many leds in your strip?
#define NUM_LEDS 9

// For led chips like WS2812, which have a data line, ground, and power, you
// just need to define DATA_PIN.
#define DATA_PIN GPIO_NUM_13

// Watchdog timeout in milliseconds
const int WATCHDOG_TIMEOUT = 8000;

// WiFi timer variables
unsigned long previousMillis = 0;
const long interval = 10000;
unsigned long currentMillis = 0;

// Search for parameter in HTTP POST request
const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "pass";

// Variables to save values from HTML form
String ssid;
String pass;
String ip = "192.168.1.200";
String gateway = "192.168.1.1";

// File paths to save input values permanently
const char *jsonWifiPath = "/wifi.json";

// Setting hostname
const char *hostname = "dice";

// Variables for Local IP address, gateway and mask
IPAddress localIP;
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0);

// "Watchdog" variable for the filesystem
boolean restart = false;

// Create web server
AsyncWebServer server(80);

// Define the array of leds
CRGB leds[NUM_LEDS];

// Dice faces
int diceZero[NUM_LEDS] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
int diceOne[NUM_LEDS] = {0, 0, 0, 0, 1, 0, 0, 0, 0};
int diceTwo[NUM_LEDS] = {1, 0, 0, 0, 0, 0, 0, 0, 1};
int diceThree[NUM_LEDS] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
int diceFour[NUM_LEDS] = {1, 0, 1, 0, 0, 0, 1, 0, 1};
int diceFive[NUM_LEDS] = {1, 0, 1, 0, 1, 0, 1, 0, 1};
int diceSix[NUM_LEDS] = {1, 0, 1, 1, 0, 1, 1, 0, 1};

// Variables for enabeling demo functions
bool stopBlink = true;
bool stopSnake = true;
bool stopDice = true;

// Initialize WiFi
bool initWiFi() {
  if (ssid == "" || ip == "") {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet)) {
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.println("Connecting to WiFi...");
  currentMillis = millis();
  previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  return true;
}

void snake(CRGB colorF, CRGB colorB) {

  for (int i = 0; i < NUM_LEDS; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      if (j == i - 1)
        leds[j] = colorF;
      else if (j == i)
        leds[j] = colorF;
      else
        leds[j] = CRGB::Black;
    }
    FastLED.show(100);
    delay(80);
  }

  for (int i = NUM_LEDS - 1; i >= 0; i--) {
    for (int j = NUM_LEDS - 1; j >= 0; j--) {
      if (j == i)
        leds[j] = colorB;
      else if (j == i + 1)
        leds[j] = colorB;
      else
        leds[j] = CRGB::Black;
    }
    FastLED.show(100);
    delay(80);
  }
  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] = CRGB::Black;
  FastLED.show();
}

CRGB randomCRGB() {
  CRGB color;

  color.r = random(0, 255);
  color.g = random(0, 255);
  color.b = random(0, 255);

  return color;
}

void transitionalBlink(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
  for (int j = 0; j <= 255; j++) {
    FastLED.show(j);
  }
  for (int j = 255; j >= 0; j--) {
    FastLED.show(j);
  }
}

void showScreen(int *number, int onTime, int offTime, bool highlight) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  delay(offTime);

  if (highlight) {
    for (int i = 0; i < 5; i++) {
      for (int i = 0; i < NUM_LEDS; i++) {
        if (number[i])
          leds[i] = CRGB::Red;
        else
          leds[i] = CRGB::Black;
      }
      FastLED.show(100);
      delay(onTime);

      for (int i = 0; i < NUM_LEDS; i++) {
        if (number[i])
          leds[i] = CRGB::Green;
        else
          leds[i] = CRGB::Black;
      }
      FastLED.show(100);
      delay(onTime);
    }
  } else {
    for (int i = 0; i < NUM_LEDS; i++) {
      if (number[i])
        leds[i] = CRGB::Red;
      else
        leds[i] = CRGB::Black;
    }
    FastLED.show(100);
    delay(onTime);
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void rollTheDice() {
  int roll = random(1, 7);
  for (int i = 0; i <= 18 + roll; i++) {
    bool highlight = i == 18 + roll;
    switch (i % 6) {
    case 0:
      showScreen(diceSix, map(i, 0, 24, 20, 400), 100, highlight);
      break;

    case 1:
      showScreen(diceOne, map(i, 0, 24, 20, 400), 100, highlight);
      break;

    case 2:
      showScreen(diceTwo, map(i, 0, 24, 20, 400), 100, highlight);
      break;

    case 3:
      showScreen(diceThree, map(i, 0, 24, 20, 400), 100, highlight);
      break;

    case 4:
      showScreen(diceFour, map(i, 0, 24, 20, 400), 100, highlight);
      break;

    case 5:
      showScreen(diceFive, map(i, 0, 24, 20, 400), 100, highlight);
      break;
    }
  }
}

CRGB stringToColor(String colorString) {
  // Convert the hex string (skipping the '#' character) to a long integer
  long hexValue = strtol(colorString.c_str() + 1, nullptr, 16);

  // Extract individual color components (red, green, and blue)
  int red = (hexValue >> 16) & 0xFF;  // Extract the red component (bits 16-23)
  int green = (hexValue >> 8) & 0xFF; // Extract the green component (bits 8-15)
  int blue = hexValue & 0xFF;         // Extract the blue component (bits 0-7)

  // Create a CRGB color using the extracted color components
  return CRGB(red, green, blue);
}

void setup() {

  // Enable the Watchdog Timer
  esp_task_wdt_init(WATCHDOG_TIMEOUT, true);
  esp_task_wdt_add(NULL);

  // Begin serial communication
  Serial.begin(115200);

  // Mount SPIFFS
  initFS();

  // Load values saved in SPIFFS
  ssid = readFileJson(SPIFFS, jsonWifiPath, "SSID");
  pass = readFileJson(SPIFFS, jsonWifiPath, "PASS");
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

  // Connect to Wi-Fi
  if (initWiFi()) {
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html");
    });

    server.serveStatic("/", SPIFFS, "/");

    server.on("/input", HTTP_POST, [](AsyncWebServerRequest *request) {
      // Check if all required parameters are present
      if (request->hasParam("blink", true)) {

        stopBlink = false;
        stopSnake = true;
        stopDice = true;

        // Send success response
        request->send(200, "text/plain", "OK");
      } else if (request->hasParam("snake", true)) {

        stopBlink = true;
        stopSnake = false;
        stopDice = true;

        // Send success response
        request->send(200, "text/plain", "OK");
      } else if (request->hasParam("dice", true)) {

        stopBlink = true;
        stopSnake = true;
        stopDice = false;

        // Send success response
        request->send(200, "text/plain", "OK");
      } else if (request->hasParam("off", true)) {
        stopBlink = true;
        stopSnake = true;
        stopDice = true;

        // Send success response
        request->send(200, "text/plain", "OK");
      } else if (request->hasParam("color", true)) {

        stopBlink = true;
        stopSnake = true;
        stopDice = true;

        Serial.println("color");
        Serial.println(request->getParam("color", true)->value());
        String colorText = request->getParam("color", true)->value();
        CRGB color = stringToColor(colorText);

        for (int i = 0; i < NUM_LEDS; i++) {
          leds[i] = color;
        }
        FastLED.show();

        // Send success response
        request->send(200, "text/plain", "OK");
      } else {
        // Send error response
        request->send(400, "text/plain", "Invalid parameters");
      }
    });

    server.begin();
  } else {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/wifimanager.html", "text/html");
    });

    server.serveStatic("/", SPIFFS, "/");

    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for (int i = 0; i < params; i++) {
        AsyncWebParameter *p = request->getParam(i);
        if (p->isPost()) {
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFileJson(SPIFFS, jsonWifiPath, "SSID", ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFileJson(SPIFFS, jsonWifiPath, "PASS", pass.c_str());
          }
        }
      }
      restart = true;
      request->send(200, "text/plain",
                    "Done. ESP will restart, and connect to your router.");
    });
    server.begin();
  }

  // Initialize ArduinoOTA with a hostname and start
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.onStart([]() { Serial.println("OTA update started"); });
  ArduinoOTA.begin();

  // FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  // LED Demo at startup
  for (int i = 0; i < 3; i++) {
    transitionalBlink(randomCRGB());
  }
  for (int i = 0; i < 2; i++) {
    snake(randomCRGB(), randomCRGB());
  }
  for (int i = 0; i < 1; i++) {
    rollTheDice();
  }
}

void loop() {
  // Handle Over-The-Air (OTA) updates for the Arduino board
  ArduinoOTA.handle();

  // Reset the Watchdog Timer to prevent a system reset
  esp_task_wdt_reset();

  // Resboot ESP after SSID and PASS were set
  if (restart) {
    delay(5000);
    ESP.restart();
  }

  while (!stopBlink) {
    transitionalBlink(randomCRGB());
  }

  while (!stopSnake) {
    snake(randomCRGB(), randomCRGB());
  }
  while (!stopDice) {
    rollTheDice();
  }
}