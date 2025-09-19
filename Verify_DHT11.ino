// #include <DHT.h>

// #define DHTPIN 4      // Set the digital pin connected to the DHT11 (GPIO4 on NodeMCU)
// #define DHTTYPE DHT11 // Set the sensor type to DHT11

// // Initialize DHT sensor.
// DHT dht(DHTPIN, DHTTYPE);

// void setup() {
//   Serial.begin(115200);
//   delay(10);
//   Serial.println();
//   Serial.println("DHT11 Test with ESP8266");
//   dht.begin();
// }

// void loop() {
//   // Wait a few seconds between measurements.
//   delay(2000);

//   // Read humidity and temperature.
//   float humidity = dht.readHumidity();
//   float temperatureC = dht.readTemperature(); // Celsius
//   float temperatureF = dht.readTemperature(true); // Fahrenheit

//   // Check if any reads failed and try again.
//   if (isnan(humidity) || isnan(temperatureC)) {
//     Serial.println("Failed to read from DHT sensor!");
//     return;
//   }

//   // Print results to Serial Monitor.
//   Serial.print("Humidity: ");
//   Serial.print(humidity);
//   Serial.print("%  ");
//   Serial.print("Temperature: ");
//   Serial.print(temperatureC);
//   Serial.print("°C / ");
//   Serial.print(temperatureF);
//   Serial.println("°F");
// }

// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>
// #include <DHT.h>

// // LCD setup
// #define LCD_ADDR 0x27
// LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// #define SOIL_PIN A0

// int soilMoisturePercent=0;
// float humidity=0;
// float temperature=0;

// // DHT setup
// #define DHTPIN D5      // Pin connected to DHT data
// #define DHTTYPE DHT11  // Or DHT22 if you’re using it
// DHT dht(DHTPIN, DHTTYPE);

// void setup() {
//   Wire.begin(D2, D1);   // SDA=D2, SCL=D1 for NodeMCU
//   lcd.init();
//   lcd.backlight();
  
//   dht.begin();

//   lcd.setCursor(0,0);
//   lcd.print("DHT Sensor Ready");
//   delay(2000);
//   lcd.clear();
// }


// void loop() {
//   float h = dht.readHumidity();
//   float t = dht.readTemperature(); // Celsius

//   int moistureValue = analogRead(SOIL_PIN);
//   soilMoisturePercent = map(moistureValue, 1024, 0, 0, 100);
//   soilMoisturePercent = constrain(moistureValue,0,100);

//   if (!isnan(h) && !isnan(t)) {
//     humidity = (humidity==0) ? h : (h*0.7+humidity*0.3);
//     temperature = (temperature==0) ? t : (t*0.7+temperature*0.3);

//     lcd.clear();
//     lcd.setCursor(0,0);
//     lcd.print("Sensor Error!");
//     delay(2000);
//     return;
//   }

//   lcd.clear();
//   lcd.setCursor(0,0);
//   lcd.print("Temp: ");
//   lcd.print(temperature);
//   lcd.print((char)223); // Degree symbol
//   lcd.print("C");

//   lcd.setCursor(0,1);
//   lcd.print("Humid: ");
//   lcd.print(humidity);
//   lcd.print("%");

//   delay(2000); // Update every 2 sec
// }


#define BLYNK_TEMPLATE_ID "TMPL3nJuK7CuD"  // Get this from Blynk Console
#define BLYNK_TEMPLATE_NAME "Smart Agriculture"
#define BLYNK_AUTH_TOKEN "VnjNOzSMempz1rEy3iMGnXbIzBFCEHTz"  // Get this from Blynk Console

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// ===== LCD Setup =====
#define LCD_ADDR 0x27
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

#define SOIL_PIN A0

int soilMoisturePercent=0;
float humidity=0;
float temperature=0;

// ===== DHT Setup =====
#define DHTPIN D5
#define DHTTYPE DHT11   // Change to DHT22 if you use DHT22
DHT dht(DHTPIN, DHTTYPE);

// ===== Wi-Fi Setup =====
char ssid[] = "Rakesh Rocky";      // Wi-Fi SSID
char pass[] = "nu1rs59j4j";  // Wi-Fi Password

// ===== Blynk Timer =====
BlynkTimer timer;

void sendToBlynk() {
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Celsius

  int moistureValue = analogRead(SOIL_PIN);
  soilMoisturePercent = map(moistureValue, 1024, 0, 0, 100);
  soilMoisturePercent = constrain(moistureValue,0,100);

  if (isnan(h) || isnan(t)) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sensor Error!");
    return;
  }else{
    humidity = (humidity==0) ? h : (h*0.7+humidity*0.3);
    temperature = (temperature==0) ? t : (t*0.7+temperature*0.3);
    Serial.println("Success!");
  }

  // Send data to Blynk
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V1, humidity);
  Blynk.virtualWrite(V2, soilMoisturePercent);
  Serial.println("✓ Data sent to Blynk");

  // Update LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print((char)223);
  lcd.print("C");

  // lcd.setCursor(0,1);
  // lcd.print("Humid: ");
  // lcd.print(humidity);
  // lcd.print("%");

  lcd.setCursor(0,1);
  lcd.print("Soil:");
  lcd.print(String(soilMoisturePercent));
}

void setup() {
  Serial.begin(9600);
  Wire.begin(D2, D1);   // SDA=D2, SCL=D1 for NodeMCU
  lcd.init();
  lcd.backlight();

  dht.begin();

  // Connect to Wi-Fi & Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Run sendToBlynk every 2 seconds
  timer.setInterval(2000L, sendToBlynk);

  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi..");
}

void loop() {
  Blynk.run();
  timer.run();
}