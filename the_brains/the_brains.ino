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
//
// The code here follows the same logic used in my RPi-Smart-Still logic processor which has been
// cooking for nearly 2 years now. We're dealing with a much smaller body of liquid here, so this
// system uses smaller timing factors when checking for temperature changes and then adjusting the
// boiler power to compensate. Just like my other project, the key is to make sure the temperature
// sensor is in solid contact with the boiler, about mid way between the bottom and the full mark.
//
// My suggestion for the installation of the temperature sensor is to use a piece of fiberglass
// to help keep it pressed against the boiler, and an extra strip of insulation to keep the cable
// from contacting the boiler. I would also recommend a healthy amount of heat sink compound on
// the sensor to guarantee better heat transfer from the boiler to the sensor. This glob of goop
// will also help keep the fiberglass insulation in place and prevent it from slipping over time.
//
// Keep in mind that if the liquid in your boiler drops below the sensor level, your temperature
// readings will not be accurate and likely erratic. The liquid acts as a heat conductor from the
// inside to the sensor, so you definitely won't want to use mode 2 or 3 for distilling water.
//------------------------------------------------------------------------------------------------
#include "Arduino_GFX_Library.h" // Standard GFX library for Arduino, built with version 1.4.9
#include "FreeSans10pt7b.h"      // https://github.com/moononournation/ArduinoFreeFontFile.git 
#include "OneWire.h"             // OneWire Network communications library
#include "DallasTemperature.h"   // Dallas Temperature DS18B20 temperature sensor library
#include "Preferences.h"         // ESP32 Flash memory read/write library
//------------------------------------------------------------------------------------------------
#define ONE_WIRE 1               // 1-Wire network pin for the DS18B20 temperature sensor
#define PIN_LCD_BL 38            // Screen backlight pin
#define PIN_POWER_ON 15          // Screen on/off pin
#define INC_BTN 0                // Value + button
#define DEC_BTN 14               // Value - button
#define SCR_OUT 3                // Analog output to the SCR controller
//------------------------------------------------------------------------------------------------
bool ActiveRun = false;          // True if there's an active distillation run
bool UpToTemp = false;           // True if the run startup has reached operating temperature
long StartTime = 0;              // Start time of the current distillation run
long LoopCounter = 0;            // Timekeeper for the loop to eliminate the need to delay it
long LastAdjustment = 0;         // Time of the last power adjustment
long Mode3Counter = 0;           // 15 minute timer for mode 3 target temperature adjustments
float TempC = 0;                 // Current temperature reading C
float TempF = 0;                 // Current temperature reading F
float Mode3Temp = 0;             // Current target temperature when running in mode 3
float Mode3Factor = 0;           // How much to increase/decrease the mode 3 target temperature
float UserTemp1 = 0;             // User selected mode 2 temperature or mode 3 start temperature
float UserTemp2 = 0;             // User selected ending temperature in mode 3
byte UserTime = 0;               // User selected distillation run time in mode 3 (hours)
byte UserPower = 0;              // User selected power level in mode 1
byte CurrentMode = 1;            // 1 = Constant Power, 2 = Constant Temp, 3 = Timed w/Temps
byte Mode3Direction = 1;         // Mode 3 temperature direction, 0 = decrease, 1 = increase
byte PowerLevel = 0;             // Current power level 0-255, (100/255) * PowerLevel = % Power
byte ActiveButton = 0;           // Currently selected touch-screen button
//------------------------------------------------------------------------------------------------
// Coordinates for touch-screen buttons (Modes 1 and 2)
int ModeX1 = 0, ModeY1 = 0, ModeX2 = 0, ModeY2 = 0;
int RunX1 = 0, RunY1 = 0, RunX2 = 0, RunY2 = 0;
int PowerX1 = 0, PowerY1 = 0, PowerX2 = 0, PowerY2 = 0;
int TempX1 = 0, TempY1 = 0, TempX2 = 0, TempY2 = 0;
// Coordinates for Mode 3 bottom row buttons
int StartX1 = 0, StartY1 = 0, StartX2 = 0, StartY2 = 0;
int EndX1 = 0, EndY1 = 0, EndX2 = 0, EndY2 = 0;
int TimeX1 = 0, TimeY1 = 0, TimeX2 = 0, TimeY2 = 0;
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

  LoopCounter = millis();
  LastAdjustment = LoopCounter;
}
//------------------------------------------------------------------------------------------------
void TempUpdate() { // Update the temperature sensor values
  DT.requestTemperatures();
  TempC = DT.getTempCByIndex(0);
  TempF = DT.getTempFByIndex(0);
  Serial.print("Temp C: "); Serial.println(TempC);
  Serial.print("Temp F: "); Serial.println(TempF);
}
//-----------------------------------------------------------------------------------------------
void PowerAdjust(byte Percent) { // Set the SCR controller to a target power percentage
  Serial.print("Power Adjust: "); Serial.println(Percent);
  LastAdjustment = millis();
  // This is an analog power controller, first set the power level to zero and rest 1 second
  analogWrite(SCR_OUT,0);
  delay(1000);
  // Then progressively adjust the power level up to the requested percentage
  if (Percent > 0) {
    PowerLevel = round(Percent * 2.55);
    float x = 2.55;
    while (x <= PowerLevel) {
      analogWrite(SCR_OUT,x);
      delay(10);
      x += 2.55;
    }
  } else {
    PowerLevel = 0;
  }
}
//-----------------------------------------------------------------------------------------------
void RunState(byte State) { // Toggle the active distillation run state
  if (State == 1 ) {
    StartTime = millis();
    ActiveRun = true;
    UpToTemp  = false;
    if (CurrentMode > 1) {
      if (CurrentMode == 3) {
        byte Range;
        if (UserTemp1 < UserTemp2) {
          Mode3Direction = 1;
          Mode3Temp = UserTemp1;
          Range = UserTemp2 - UserTemp1;
        } else {
          Mode3Direction = 0;
          Mode3Temp = UserTemp2;
          Range = UserTemp1 - UserTemp2;
        }
        Mode3Factor  = Range / (UserTime * 4);
        Mode3Counter = millis();
      }
      PowerAdjust(100);
    } else {
      PowerAdjust(UserTemp1);
    }
  } else {
    ActiveRun = false;
    PowerAdjust(0);
  }
}
//-----------------------------------------------------------------------------------------------
bool ButtonTapped(int Xpos, int Ypos, int X1, int Y1, int X2, int Y2) { // Button coordinate evaluator
  if ((Xpos >= X1) && (Xpos <= X2) && (Ypos >= Y1) && (Ypos <= Y2)) {
    return true;
  } else {
    return false;
  }
}
//-----------------------------------------------------------------------------------------------
void ProcessTouch(int Xpos, int Ypos) { // Handle touch-screen inputs
  // Process start/stop button presses and exit
  if (ButtonTapped(Xpos,Ypos,RunX1,RunY1,RunX2,RunY2)) {
    if (ActiveRun) {
      RunState(0);
    } else {
      RunState(1);
    }
    return;
  }
  // Process top row button presses

  // Process bottom row button presses
  if (CurrentMode == 3) {

  } else {

  }
}
//-----------------------------------------------------------------------------------------------
void ProcessButton(byte WhichOne) { // Handle increment/decrement button inputs
  
}
//-----------------------------------------------------------------------------------------------
void loop() {
  int CurrentPercent = round(0.392156863 * PowerLevel);
  long CurrentTime = millis();
  if (CurrentTime > 4200000000) {
    // Reboot the system if we're reaching the maximum long integer value of CurrentTime
    ESP.restart();
  }
  // Check for touch-screen keypresses and handle as necessary

  // Check for Value+ keypresses and handle as necessary
  if ((digitalRead(INC_BTN) == 0) && (! ActiveRun)) {
    Serial.println("+ button pressed");
    while (digitalRead(INC_BTN) == 0) delay(10);
  }
  // Check for Value- keypresses and handle as necessary
  if ((digitalRead(DEC_BTN) == 0) && (! ActiveRun)) {
    Serial.println("- button pressed");
    while (digitalRead(DEC_BTN) == 0) delay(10);
  }
  // Perform status updates every second without using a delay(1000) in the loop
  // The delay function can't be used in this loop due to the buttons and touch-screen
  if (CurrentTime - LoopCounter >= 1000) {
    Serial.println("Running Status Updates");
    TempUpdate(); // Read the DS18B20 temperature
    if (ActiveRun) {
      if (CurrentMode > 1) { // Mode 1 is constant power, no temperature management
        if (! UpToTemp) {
          if (TempC >= UserTemp1) { // Minimum operating temperature has been reached
            UpToTemp = true;
            PowerAdjust(70); // Fall back to 70% power and begin temperature management
          }
        } else {
          if (CurrentMode == 2) { // Constant temperature
            if (CurrentTime - LastAdjustment >= 60000) { // Only make power adjustments once per minute
              // Temperature is managed to +/- 1 degree C
              if (TempC >= (UserTemp1 + 1)) { // Over temperature
                CurrentPercent -= 1;
                if (CurrentPercent < 0) CurrentPercent = 0;
                PowerAdjust(CurrentPercent); // Decrease power 1%
              } else if (TempC <= (UserTemp1 - 1)) { // Under temperature
                CurrentPercent += 1;
                if (CurrentPercent > 100) CurrentPercent = 100;
                PowerAdjust(CurrentPercent); // Increase power 1%
              }
            }
          } else { // Timed distillation run with progressive temperature adjustment
            if (CurrentTime - StartTime < UserTime) {
              if (CurrentTime - Mode3Counter >= 900000) { // Adjust the target temperature every 15 minutes
                if (Mode3Direction == 1) {
                  Mode3Temp += Mode3Factor; // Increase the target temperature
                } else {
                  Mode3Temp -= Mode3Factor; // Decrease the target temperature
                }
              }
              if (CurrentTime - LastAdjustment >= 60000) { // Only make power adjustments once per minute
                // Temperature is managed to +/- 1 degree C
                if (TempC >= (Mode3Temp + 1)) { // Over temperature
                  CurrentPercent -= 1;
                  if (CurrentPercent < 0) CurrentPercent = 0;
                  PowerAdjust(CurrentPercent); // Decrease power 1%
                } else if (TempC <= (Mode3Temp - 1)) { // Under temperature
                  CurrentPercent += 1;
                  if (CurrentPercent > 100) CurrentPercent = 100;
                  PowerAdjust(CurrentPercent); // Increase power 1%
                }
              }
            } else { // Distillation run time has expired, shut down
              RunState(0);
            }
          }
        }
      }
    }
    LoopCounter = CurrentTime;
  }
}
//------------------------------------------------------------------------------------------------