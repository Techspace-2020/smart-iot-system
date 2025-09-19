#define BLYNK_TEMPLATE_ID "TMPL3nJuK7CuD"  // Get this from Blynk Console
#define BLYNK_TEMPLATE_NAME "Smart Agriculture"
#define BLYNK_AUTH_TOKEN "VnjNOzSMempz1rEy3iMGnXbIzBFCEHTz"  // Get this from Blynk Console

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// ===== LCD Setup =====
#define LCD_ADDR 0x27
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

#define SOIL_PIN A0
#define RELAY_PIN D3      // D3 - Relay control pin

int soilMoisturePercent=0;
int dryVal = 990;   // calibrate in dry soil
int wetVal = 400;   // calibrate in wet soil

// ===== Wi-Fi Setup =====
char ssid[] = "Rakesh Rocky";      // Wi-Fi SSID
char pass[] = "nu1rs59j4j";  // Wi-Fi Password

// ===== Blynk Timer =====
BlynkTimer timer;

void sendToBlynk() {
  int moistureValue = analogRead(SOIL_PIN);
  soilMoisturePercent = map(moistureValue,dryVal,wetVal,0,100);
  
  if(soilMoisturePercent<0) soilMoisturePercent=0;
  if(soilMoisturePercent>100) soilMoisturePercent=100;

  Serial.print("Mositure level:");
  Serial.print(soilMoisturePercent);
  Serial.print("%");

  // Control pump: turn ON if soil < 30%, OFF if > 60%
  if (soilMoisturePercent < 30) {
    digitalWrite(RELAY_PIN, HIGH);  // turn pump ON
    Serial.println("Pump ON");
    Blynk.logEvent("low_soil_moisture", "⚠️ ಮಣ್ಣು ಒಣಗಿದೆ, ನೀರು ಹಾಯಿಸಲಾಗುತ್ತಿದೆ!");
  } 
  else if (soilMoisturePercent > 60) {
    digitalWrite(RELAY_PIN, LOW); // turn pump OFF
    Serial.println("Pump OFF");
  }

  // Send data to Blynk
  Blynk.virtualWrite(V2, soilMoisturePercent);
  //Serial.println("✓ Data sent to Blynk");

  // Update LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Soil:");
  lcd.print(String(soilMoisturePercent));
}

void setup() {   
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  pinMode(RELAY_PIN, OUTPUT);


  // Connect to Wi-Fi & Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Run sendToBlynk every 2 seconds
  timer.setInterval(2000L, sendToBlynk);
  lcd.print("Connecting WiFi..");
  if(WiFi.status()==WL_CONNECTED){
    lcd.setCursor(0,0);
    lcd.print("WiFi Connected!");
  }else{
    lcd.setCursor(0,0);
    lcd.print("Not Connected");
  }

}

void loop() {
  Blynk.run();
  timer.run();
}



// int soilPin = A0;  // Soil moisture sensor analog pin
// int soilValue = 0; // Variable to store sensor value

// int dryVal = 990;   // calibrate in air
// int wetVal = 400;   // calibrate in water

// void setup() {
//   Serial.begin(9600);
// }

// void loop() {
//   soilValue = analogRead(soilPin); // Read analog value

//   // Map raw value to 0-100% (inverted if needed)
//   int moisturePercent = map(soilValue,dryVal,wetVal,0,100);

//   // clamp values between 0 and 100
//   if (moisturePercent < 0) moisturePercent = 0;
//   if (moisturePercent > 100) moisturePercent = 100;

//   Serial.print("Soil Moisture: ");
//   Serial.print(moisturePercent);
//   Serial.println("%");

//   delay(2000);  // Read every 2 seconds
// }

// int getAverageMoisture() {
//   long sum = 0;
//   for (int i = 0; i < 10; i++) {
//     sum += analogRead(soilPin);
//     delay(10);
//   }
//   return sum / 10;
// }



