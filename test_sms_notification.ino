#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

const char* ssid = "JioFiber4G";
const char* password = "Animal1234";

const char* apiKey = "EnmGJ7TYLrGM";
const char* templateID = "101";
const char* mobileNumber = "919535234180";
const char* var1 = "Tempearture level";
const char* var2 = "20";

const char* templateIDforSoil = "101";



void sendSMS() {
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
  const char* var2 = "10";
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


void setup() {
 Serial.begin(9600);
 WiFi.begin(ssid,password);
 Serial.print("Connecting to wifi");
 while(WiFi.status() != WL_CONNECTED){
   delay(500);
   Serial.print(".");
 }
 Serial.println("\nConnected!");
 sendSMS();
 sendSoilMoistureSMS();
}
void loop() {
 // put your main code here, to run repeatedly:
}