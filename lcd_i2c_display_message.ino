#include <LCD-I2C.h>
#include <Wire.h>

LCD_I2C lcd(0x27, 16, 2); // Default address of most PCF8574 modules, change according

void setup() {
    Wire.begin();
    lcd.begin(&Wire);
    lcd.display();
    lcd.backlight();
}
void loop()
{
  lcd.setCursor(0, 0);
  lcd.print("WELCOME TO MIET");
  delay(1000);

  lcd.setCursor(0, 1);
  lcd.print("5TH ECE IOT LAB");
  delay(1000);
}
