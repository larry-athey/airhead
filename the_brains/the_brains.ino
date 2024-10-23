//------------------------------------------------------------------------------------------------
// Airhead - Air Still Brain | (CopyLeft) 2024-Present | Larry Athey (https://panhandleponics.com)
//
// This code requires the LilyGo T-Display-S3 ESP32 board. You will first need to properly set up
// your Arduino IDE to upload to that board. See: https://github.com/Xinyuan-LilyGO/T-Display-S3
// Make sure that you only install the Espressif ESP32 v2.0.14 board library, no version 3.x stuff
//------------------------------------------------------------------------------------------------
#include "Arduino_GFX_Library.h" // Standard GFX library for Arduino, built with version 1.4.9
#include "FreeSans10pt7b.h"      // https://github.com/moononournation/ArduinoFreeFontFile.git 
#include "OneWire.h"             // OneWire Network communications library
#include "DallasTemperature.h"   // Dallas Temperature DS18B20 temperature sensor library
#include "Preferences.h"         // ESP32 Flash memory read/write library
//------------------------------------------------------------------------------------------------
#define ONE_WIRE 1               // 1-Wire network pin for DS18B20 temperature sensor
#define PIN_LCD_BL 38            // Screen backlight pin
#define PIN_POWER_ON 15          // Screen on/off pin
//------------------------------------------------------------------------------------------------
Arduino_DataBus *bus = new Arduino_ESP32LCD8(7 /* DC */, 6 /* CS */, 8 /* WR */, 9 /* RD */, 39 /* D0 */, 40 /* D1 */, 41 /* D2 */, 42 /* D3 */,
                                             45 /* D4 */, 46 /* D5 */, 47 /* D6 */, 48 /* D7 */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 5 /* RST */, 0 /* rotation */, true /* IPS */, 170 /* width */, 320 /* height */, 35 /* col offset 1 */,
                                           0 /* row offset 1 */, 35 /* col offset 2 */, 0 /* row offset 2 */);
//------------------------------------------------------------------------------------------------
OneWire oneWire(ONE_WIRE);
DallasTemperature DT(&oneWire);
Preferences preferences;
//------------------------------------------------------------------------------------------------
void setup() {
  DT.begin();
  pinMode(PIN_POWER_ON,OUTPUT);
  digitalWrite(PIN_POWER_ON,HIGH);
  ledcSetup(0,2000,8);
  ledcAttachPin(PIN_LCD_BL,0);
  ledcWrite(0,255); // Screen brightness (0-255)

  gfx->begin();
  gfx->setRotation(1);
  gfx->fillScreen(BLACK);

}
//------------------------------------------------------------------------------------------------
void loop() {


}
//------------------------------------------------------------------------------------------------