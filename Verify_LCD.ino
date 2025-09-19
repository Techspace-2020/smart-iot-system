#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDR 0x27  // your detected address
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

void setup() {
  Wire.begin(D2, D1);  // NodeMCU I2C pins: SDA=D2, SCL=D1
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Hello NodeMCU!");
  lcd.setCursor(0, 1);
  lcd.print("LCD 16x2 I2C OK");
}

void loop() {
  // Optional: update text every second
  static unsigned long last = 0;
  static int counter = 0;

  if (millis() - last > 1000) {
    last = millis();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Counter: ");
    lcd.print(counter++);
    lcd.setCursor(0,1);
    lcd.print("Time: ");
    lcd.print(millis()/1000);
  }
}
