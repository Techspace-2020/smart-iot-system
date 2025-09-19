/*
 * Smart Agriculture IoT Solution - Using Wire Library for LCD
 * No external LCD library required - uses built-in Wire library
 * Compatible with all ESP8266 boards
 */

// Blynk credentials (MUST be defined BEFORE including Blynk library)
#define BLYNK_TEMPLATE_ID "TMPL3nJuK7CuD"  // Get this from Blynk Console
#define BLYNK_TEMPLATE_NAME "Smart Agriculture"
#define BLYNK_AUTH_TOKEN "VnjNOzSMempz1rEy3iMGnXbIzBFCEHTz"  // Get this from Blynk Console

// Comment this line to disable debug prints
#define BLYNK_PRINT Serial

// Include required libraries
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <string>

// WiFi credentials
// char ssid[] = "YOUR_WIFI_SSID";     // Replace with your WiFi name
// char pass[] = "YOUR_WIFI_PASSWORD"; // Replace with your WiFi password
const char* ssid = "Rakesh Rocky";
const char* password = "nu1rs59j4j";

const char* apiKey = "qfNY3LaMi3zi";
const char* templateID = "101";
const char* mobileNumber = "919535234180";


// Pin definitions
#define DHT_PIN D5        // D4 - DHT11 data pin
#define DHT_TYPE DHT11   // DHT sensor type
#define SOIL_PIN A0      // A0 - Soil moisture sensor
#define RELAY_PIN D3      // D3 - Relay control pin
#define LED_PIN D0       // D0 - Status LED pin
#define SDA_PIN D2        // D2 - I2C SDA for LCD
#define SCL_PIN D1        // D1 - I2C SCL for LCD

// LCD I2C Configuration
#define LCD_ADDRESS 0x27  // Most common I2C address
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// LCD Commands for I2C
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// LCD Flags
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0b00000100  // Enable bit
#define Rw 0b00000010  // Read/Write bit
#define Rs 0b00000001  // Register select bit

// Sensor object
DHT dht(DHT_PIN, DHT_TYPE);

// Variables
float temperature = 0;
float humidity = 0;
int soilMoisturePercent = 0;
bool pumpStatus = false;
bool autoMode = true;
bool systemInitialized = false;
bool lcdInitialized = false;

//Soil default calibration
int dryVal = 990;   // calibrate in dry soil
int wetVal = 400;   // calibrate in wet soil

// LCD display variables
int currentScreen = 0;
unsigned long screenChangeTime = 0;
const unsigned long SCREEN_DURATION = 4000;  // 4 seconds per screen

// Thresholds
const int DRY_SOIL_THRESHOLD = 30;
const float HIGH_TEMP_THRESHOLD = 35;
const float LOW_HUMIDITY_THRESHOLD = 40;

// Timing variables
unsigned long previousMillis = 0;
const long interval = 5000;

// Blynk timer
BlynkTimer timer;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("**Smart Agriculture System**");
  
  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, HIGH);
  
  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Initialize LCD
  initializeLCD();
  
  // Initialize DHT sensor
  dht.begin();
  Serial.println("DHT11 sensor initialized");
  
  if (lcdInitialized) {
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("Smart IOT APP");
    lcdSetCursor(0, 1);
    lcdPrint("Initializing...");
  }
  
  delay(2000);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  if (lcdInitialized) {
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("Connecting WiFi...");
    lcdSetCursor(0, 1);
    lcdPrint(String(ssid).substring(0, 16));
  }

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Blynk.config(BLYNK_AUTH_TOKEN);   // Configure only
    Blynk.connect();                  // Try connect once

    if (lcdInitialized) {
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("WiFi Connected!");
      lcdSetCursor(0, 1);
      lcdPrint(WiFi.localIP().toString());
    }
  } else {
    Serial.println("\nWiFi Failed!");
    if (lcdInitialized) {
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("WiFi Failed!");
    }
  }
  
  if (Blynk.connected()) {
    Serial.println("\nConnected to Blynk IOT!");
    if (lcdInitialized) {
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("Blynk Connected");
      lcdSetCursor(0, 1);
      lcdPrint("IP:" + WiFi.localIP().toString().substring(0, 12));
      delay(2000);
    }
  } else {
    Serial.println("Failed to connect!");
    if (lcdInitialized) {
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("Connection Failed to Blynk IOT");
      lcdSetCursor(0, 1);
      lcdPrint("Please check the Settings");
      delay(3000);
    }
  }
  
  // Setup timers
  timer.setInterval(10000L, sendSensorData);
  timer.setInterval(5000L, checkThresholds);
  timer.setInterval(1000L, updateLCDDisplay);
  
  digitalWrite(LED_PIN, LOW);
  systemInitialized = true;
  
  Serial.println("System ready!");
  
  if (lcdInitialized) {
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("System Ready!");
    lcdSetCursor(0, 1);
    lcdPrint("Monitoring...");
    delay(1500);
  }
}

void loop() {
  Blynk.run();
  timer.run();
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    readSensors();
    printSensorData();
    updateStatusLED();
  }
}

// LCD Functions using Wire library
void initializeLCD() {
  Serial.println("Initializing LCD Communication via I2C...");
  
  // Try different I2C addresses
  byte addresses[] = {0x27, 0x3F, 0x26, 0x20};
  
  for (int i = 0; i < 4; i++) {
    Wire.beginTransmission(addresses[i]);
    if (Wire.endTransmission() == 0) {
      Serial.printf("LCD found at address 0x%02X\n", addresses[i]);
      
      // Initialize LCD
      lcdWrite4bits(0x30);
      delayMicroseconds(4500);
      lcdWrite4bits(0x30);
      delayMicroseconds(4500);
      lcdWrite4bits(0x30);
      delayMicroseconds(150);
      lcdWrite4bits(0x20);
      
      lcdCommand(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
      lcdCommand(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
      lcdCommand(LCD_CLEARDISPLAY);
      delayMicroseconds(2000);
      lcdCommand(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
      lcdCommand(LCD_RETURNHOME);
      delayMicroseconds(2000);
      
      lcdBacklight();
      lcdInitialized = true;
      
      // Test LCD
      lcdSetCursor(0, 0);
      // lcdPrint("LCD communication successfull!!");
      delay(1000);
      
      return;
    }
  }
  
  Serial.println("LCD not found! Continuing without LCD.");
  lcdInitialized = false;
}

void lcdCommand(uint8_t value) {
  lcdSend(value, 0);
}

void lcdWrite(uint8_t value) {
  lcdSend(value, Rs);
}

void lcdSend(uint8_t value, uint8_t mode) {
  uint8_t highnib = value & 0xf0;
  uint8_t lownib = (value << 4) & 0xf0;
  lcdWrite4bits((highnib) | mode);
  lcdWrite4bits((lownib) | mode);
}

void lcdWrite4bits(uint8_t value) {
  lcdExpanderWrite(value);
  lcdPulseEnable(value);
}

void lcdExpanderWrite(uint8_t data) {
  Wire.beginTransmission(LCD_ADDRESS);
  Wire.write((int)(data) | LCD_BACKLIGHT);
  Wire.endTransmission();
}

void lcdPulseEnable(uint8_t data) {
  lcdExpanderWrite(data | En);
  delayMicroseconds(1);
  lcdExpanderWrite(data & ~En);
  delayMicroseconds(50);
}

void lcdBacklight() {
  lcdExpanderWrite(LCD_BACKLIGHT);
}

void lcdNoBacklight() {
  lcdExpanderWrite(LCD_NOBACKLIGHT);
}

void lcdClear() {
  lcdCommand(LCD_CLEARDISPLAY);
  delayMicroseconds(2000);
}

void lcdHome() {
  lcdCommand(LCD_RETURNHOME);
  delayMicroseconds(2000);
}

void lcdSetCursor(uint8_t col, uint8_t row) {
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if (row > LCD_ROWS) {
    row = LCD_ROWS - 1;
  }
  lcdCommand(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void lcdPrint(String text) {
  for (int i = 0; i < text.length(); i++) {
    lcdWrite(text[i]);
  }
}

// Sensor and system functions
void readSensors() {
  float newHumidity = dht.readHumidity();
  float newTemperature = dht.readTemperature();
  
  if (isnan(newHumidity) || isnan(newTemperature)) {
    Serial.println("DHT read error!");
    lcdSetCursor(0, 0);
    lcdPrint("DHT read error!");
    return;
  } else {
    humidity = (humidity == 0) ? newHumidity : (humidity * 0.7 + newHumidity * 0.3);
    temperature = (temperature == 0) ? newTemperature : (temperature * 0.7 + newTemperature * 0.3);
  }
  
  // ✅ Soil sensor read
  int soilValue = analogRead(SOIL_PIN);

  // Correct mapping: wet=100%, dry=0%
  soilMoisturePercent = map(soilValue, wetVal, dryVal, 100, 0);
  soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);

  Serial.printf("[Soil RAW:%d → %d%%]\n", soilValue, soilMoisturePercent);

  //soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);
  // if(soilMoisturePercent<0) soilMoisturePercent=0;
  // if(soilMoisturePercent>100) soilMoisturePercent=100;
}

void printSensorData() {
  Serial.println("--- Sensor Data ---");
  Serial.printf("Temperature: %.1f°C\n", temperature);
  Serial.printf("Humidity: %.1f%%\n", humidity);
  Serial.printf("Soil: %d%% (Raw: %d)\n", soilMoisturePercent, analogRead(SOIL_PIN));
  Serial.printf("Pump: %s | Mode: %s\n", pumpStatus ? "ON" : "OFF", autoMode ? "AUTO" : "MANUAL");
  Serial.println("------------------");
}

void updateLCDDisplay() {
  if (!lcdInitialized) return;
  
  unsigned long currentTime = millis();
  
  if (currentTime - screenChangeTime >= SCREEN_DURATION) {
    currentScreen = (currentScreen + 1) % 3;
    screenChangeTime = currentTime;
  }
  
  lcdClear();
  
  switch (currentScreen) {
    case 0:  // Main data
      lcdSetCursor(0, 0);
      lcdPrint("T:" + String(temperature, 1) + " H:" + String(humidity, 1) + "%");
      lcdSetCursor(0, 1);
      lcdPrint("Soil:" + String(soilMoisturePercent) + "% P:" + (pumpStatus ? "ON" : "OFF"));
      break;
      
    case 1:  // System status
      lcdSetCursor(0, 0);
      lcdPrint("Mode: " + String(autoMode ? "AUTO" : "MANUAL"));
      lcdSetCursor(0, 1);
      lcdPrint("Thres:" + String(DRY_SOIL_THRESHOLD) + "% Pump:" + (pumpStatus ? "Y" : "N"));
      break;
      
    case 2:  // Network
      lcdSetCursor(0, 0);
      lcdPrint("WiFi: " + String(WiFi.status() == WL_CONNECTED ? "OK" : "ERROR"));
      lcdSetCursor(0, 1);
      lcdPrint("Blynk: " + String(Blynk.connected() ? "CONNECTED" : "ERROR"));
      break;
  }
}

void sendSensorData() {
  if (!systemInitialized) return;
  
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, soilMoisturePercent);
  Blynk.virtualWrite(V3, pumpStatus ? 1 : 0);
  
  String status = "T:" + String(temperature, 1) + "°C H:" + String(humidity, 1) + 
                  "% S:" + String(soilMoisturePercent) + "% P:" + (pumpStatus ? "ON" : "OFF");
  Blynk.virtualWrite(V4, status);
  Blynk.virtualWrite(V5, autoMode ? "AUTO MODE" : "MANUAL MODE");
  
  Serial.println("✓ Data sent to Blynk");

  // if(!isnan(temperature) && !isnan(humidity)){
  //     sendSMS();
  // }
  // if(soilMoisturePercent<30){
  //     sendSoilMoistureSMS();
  // }
}

void checkThresholds() {
  //Serial.printf("[DEBUG] AutoMode:%d | Soil:%d%% | Pump:%d\n", autoMode, soilMoisturePercent, pumpStatus);
  if (!autoMode || !systemInitialized) return;
  
  if (soilMoisturePercent < DRY_SOIL_THRESHOLD && !pumpStatus) {
    Serial.printf("Soil low (%d%%), starting pump\n", soilMoisturePercent);
    controlPump(true);
    
    if (lcdInitialized) {
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("ALERT: Low Soil!");
      lcdSetCursor(0, 1);
      lcdPrint("Starting Pump...");
      delay(2000);
    }
    
    Blynk.logEvent("low_soil_moisture","⚠️ಮಣ್ಣು ಒಣಗಿದೆ, ನೀರು ಹಾಯಿಸಲಾಗುತ್ತಿದೆ!" + String(soilMoisturePercent) + "%");
  }
  if (soilMoisturePercent > (DRY_SOIL_THRESHOLD + 5) && pumpStatus) {
    Serial.printf("[DEBUG] Soil:%d%% reached stop threshold, stopping pump\n", soilMoisturePercent);
    Serial.printf("Soil OK (%d%%), stopping pump\n", soilMoisturePercent);
    controlPump(false);
  }
  
  if (temperature > HIGH_TEMP_THRESHOLD) {
    Serial.printf("High temp: %.1f°C\n", temperature);
    Blynk.logEvent("high_temperature", "⚠️ತಾಪಮಾನ ಹೆಚ್ಚಾಗಿದೆ: " + String(temperature, 1) + "°C");
  }
  
  if (humidity < LOW_HUMIDITY_THRESHOLD && humidity > 0) {
    Serial.printf("Low humidity: %.1f%%\n", humidity);
    Blynk.logEvent("low_humidity", "⚠️ ತೇವಾಂಶ ಕಡಿಮೆಯಾಗಿದೆ: " + String(humidity, 1) + "%");
  }
}

void controlPump(bool state) {
  pumpStatus = state;
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  
  Serial.printf("Pump %s\n", state ? "ON" : "OFF");
  Blynk.virtualWrite(V3, state ? 1 : 0);
  
  if (lcdInitialized) {
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("PUMP: " + String(state ? "ON" : "OFF"));
    lcdSetCursor(0, 1);
    lcdPrint(state ? "Watering..." : "Stopped");
    delay(1500);
  }
  
  if (state) {
    Blynk.logEvent("pump_on", "💧ಮೋಟಾರ್ ಪ್ರಾರಂಭವಾಗಿದೆ (ಪಂಪ್ ON)");
  } else {
    Blynk.logEvent("pump_off", "💧ಮೋಟಾರ್ ನಿಂತಿದೆ (ಪಂಪ್ OFF)");
  }
}

void updateStatusLED() {
  static bool ledState = false;
  static unsigned long ledTimer = 0;
  
  unsigned long interval = 1000;
  
  if (!WiFi.isConnected()) interval = 200;
  else if (!Blynk.connected()) interval = 500;
  else if (pumpStatus) interval = 300;
  else interval = 2000;
  
  if (millis() - ledTimer > interval) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    ledTimer = millis();
  }
}



void sendSMS() {
  const char* var1 = "Tempearture level";
  const char* var2 = ([](float t){ static char buf[10]; dtostrf(t, 0, 1, buf); return buf; })(temperature);
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client; // Use WiFiClientSecure for HTTPS connections
    client.setInsecure();    // Skip certificate validation (not secure but works for development)
    HTTPClient http;
    // Build the API URL with the template ID
    String apiUrl = "https://www.circuitdigest.cloud/send_sms?ID=" + String(templateID);
    // Start the HTTPS connection with WiFiClientSecure
    http.begin(client, apiUrl);
    http.addHeader("Authorization", apiKey);
    http.addHeader("Content-Type", "application/json");
    // Create the JSON payload with SMS details
    String payload = "{\"mobiles\":\"" + String(mobileNumber) + "\",\"var1\":\"" + String(var1) + "\",\"var2\":\"" + String(var2) + "\"}";
    // Send POST request
    int httpResponseCode = http.POST(payload);
    // Check response
    if (httpResponseCode == 200) {
      Serial.println("SMS sent successfully!");
      Serial.println(http.getString());
    } else {
      Serial.print("Failed to send SMS. Error code: ");
      Serial.println(httpResponseCode);
      Serial.println("Response: " + http.getString());
    }
    http.end(); // End connection
  } else {
    Serial.println("WiFi not connected!");
  }
  }

void sendSoilMoistureSMS() {
  const char* templateIDforSoil = "101";
  const char* var1 = "Soil moisture level";
  String data = String(soilMoisturePercent);
  const char* var2 = data.c_str();
  
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client; // Use WiFiClientSecure for HTTPS connections
    client.setInsecure();    // Skip certificate validation (not secure but works for development)
    HTTPClient http;
    // Build the API URL with the template ID
    Serial.println("Inside soil moisture loop");
    String apiUrl = "https://www.circuitdigest.cloud/send_sms?ID=" + String(templateIDforSoil);
    // Start the HTTPS connection with WiFiClientSecure
    http.begin(client, apiUrl);
    http.addHeader("Authorization", apiKey);
    http.addHeader("Content-Type", "application/json");
    // Create the JSON payload with SMS details
    String payload = "{\"mobiles\":\"" + String(mobileNumber) + "\",\"var1\":\"" + String(var1) + "\",\"var2\":\"" + String(var2) + "\"}";
    // Send POST request
    int httpResponseCode = http.POST(payload);
    // Check response
    if (httpResponseCode == 200) {
      Serial.println("SMS sent successfully!");
      Serial.println(http.getString());
    } else {
      Serial.print("Failed to send SMS. Error code: ");
      Serial.println(httpResponseCode);
      Serial.println("Response: " + http.getString());
    }
    http.end(); // End connection
  } else {
    Serial.println("WiFi not connected!");
  }
  }

// Blynk handlers
BLYNK_WRITE(V3) {
  int buttonState = param.asInt();
  
  if (buttonState == 1) {
    Serial.println("Manual pump ON from app");
    autoMode = false;
    controlPump(true);
    Blynk.virtualWrite(V5, "MANUAL MODE");
    
    timer.setTimeout(5000L, []() {
      controlPump(false);
      autoMode = true;
      Serial.println("Back to auto mode");
      Blynk.virtualWrite(V5, "AUTO MODE");
    });
  } else {
    Serial.println("Manual pump OFF from app");
    controlPump(false);
  }
}

BLYNK_CONNECTED() {
  Serial.println("✓ Blynk connected");
  Blynk.syncAll();
  Blynk.virtualWrite(V5, autoMode ? "AUTO MODE" : "MANUAL MODE");
}

BLYNK_DISCONNECTED() {
  Serial.println("✗ Blynk disconnected");
}
