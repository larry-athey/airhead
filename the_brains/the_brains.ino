//------------------------------------------------------------------------------------------------
// Airhead - Air Still Brain | (CopyLeft) 2024-Present | Larry Athey (https://panhandleponics.com)
//
// This code requires the LilyGo T-Display-S3 ESP32 board. You will first need to properly set up
// your Arduino IDE to upload to that board. See: https://github.com/Xinyuan-LilyGO/T-Display-S3
// Make sure that you only install the Espressif ESP32 v2.0.14 board library since the 3.x stuff
// is not backward compatible and will throw syntax errors all over the place. (Nice fkin upgrade)
//
// This code is written for a 320x170 touch screen. If you're wanting to use a different ESP32 and
// a larger screen, you'll need to modify a lot of coordinate values in order to use more than the
// upper left corner of a larger screen. Honestly, for as little information that's on the screen,
// I don't see any point in using a larger screen or bloating the code with a built-in web API.
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
#define INC_BTN 0                // Value + button
#define DEC_BTN 14               // Value - button
#define SCR_OUT 3                // Analog output to the SCR controller
//------------------------------------------------------------------------------------------------
bool ActiveRun = false;          // True if there's an active distillation run
long StartTime = 0;              // Start time from millis() of the current distillation run
float TempC = 0;                 // Current temperature reading C
float TempF = 0;                 // Current temperature reading F
byte PowerLevel = 0;             // Current power level 0-255, (100/255) * PowerLevel = % Power
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
  Serial.begin(9600);
  while (! Serial) delay(10);
  Serial.println("");
  DT.begin();

  pinMode(SCR_OUT,OUTPUT);
  analogWrite(SCR_OUT,PowerLevel);

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

  if (digitalRead(INC_BTN) == 0) {
    Serial.println("+ button pressed");
    while (digitalRead(INC_BTN) == 0) delay(10);
  }

  if (digitalRead(DEC_BTN) == 0) {
    Serial.println("- button pressed");
    while (digitalRead(DEC_BTN) == 0) delay(10);
  }

  delay(100);
}
//------------------------------------------------------------------------------------------------