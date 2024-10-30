//------------------------------------------------------------------------------------------------
// Airhead - Air Still Brain | (CopyLeft) 2024-Present | Larry Athey (https://panhandleponics.com)
//
// This code requires the LilyGo T-Display-S3 ESP32 board. You will first need to properly set up
// your Arduino IDE to upload to that board. See: https://github.com/Xinyuan-LilyGO/T-Display-S3
// Make sure that you only install the Espressif ESP32 v2.0.14 board library since the 3.x stuff
// is not backward compatible and will throw syntax errors all over the place. (Nice fkin upgrade)
//
// This code is written for a 320x170 touch-screen. If you're wanting to use a different ESP32 and
// a larger screen, you'll need to modify a lot of coordinate values in order to use more than the
// upper left corner of a larger screen. Honestly, for as little information that's on the screen,
// I don't see any point in using a larger screen or making it mobile web browser capable.
//
// The code here follows the same logic used in my RPi-Smart-Still logic processor which has been
// cooking for nearly 2 years now. We're dealing with a much smaller body of liquid here, so this
// system uses smaller timing factors when checking for temperature changes and then adjusting the
// boiler power to compensate. Just like my other project, the key is to make sure the temperature
// sensor is in solid contact with the boiler, about mid way between the bottom and the full mark.
//
// My suggestion for the installation of the temperature sensor is to use one in a TO-92 case and
// attach it in place of the original snap switch using a TO-92 chassis heat sink mount. If those
// are hard to find, any other strip of metal with holes in the ends for the mounting bolts works
// just fine. Be sure to add heat sink compound for reliable heat transfer. If possible, use wire
// with silicone insulation for the connections and heat shrink the leads to prevent any shorts.
//------------------------------------------------------------------------------------------------
#include "Arduino_GFX_Library.h" // Standard GFX library for Arduino, built with version 1.4.9
#include "FreeSans10pt7b.h"      // https://github.com/moononournation/ArduinoFreeFontFile.git 
#include "OneWire.h"             // OneWire Network communications library
#include "DallasTemperature.h"   // Dallas Temperature DS18B20 temperature sensor library
#include "Preferences.h"         // ESP32 Flash memory read/write library
#include "Wire.h"                // I2C communications library for touch-screen interface
#define TOUCH_MODULES_CST_SELF   // Tell TouchLib.h to use the CST816 chip routines
#include "TouchLib.h"            // LilyGo touch-screen interface library
//------------------------------------------------------------------------------------------------
#define ONE_WIRE 13              // 1-Wire network pin for the DS18B20 temperature sensor
#define SCR_OUT 1                // Analog output to the SCR controller
#define SCL 17                   // I2C clock pin
#define SDA 18                   // I2C data pin
#define SCREEN_BACKLIGHT 38      // Screen backlight LED pin
#define SCREEN_POWER_ON 15       // Screen power on/off pin
#define INC_BTN 0                // Value + button
#define DEC_BTN 14               // Value - button
#define TOUCH_INT 16             // CPU interrupt monitor pin for touch-screen input hook
#define TOUCH_RES 21             // Reset pin for touch-screen controller chip
//------------------------------------------------------------------------------------------------
bool ActiveRun = false;          // True if there's an active distillation run
bool UpToTemp = false;           // True if the run startup has reached operating temperature
bool GotInterrupt = false;       // True if touch input has been detected on the screen
long StartTime = 0;              // Start time of the current distillation run
long LoopCounter = 0;            // Timekeeper for the loop to eliminate the need to delay it
long LastAdjustment = 0;         // Time of the last power adjustment
long Mode3Counter = 0;           // 15 minute timer for mode 3 target temperature adjustments
float TempC = 0;                 // Current temperature reading C
float TempF = 0;                 // Current temperature reading F
float Mode3Temp = 0;             // Current target temperature when running in mode 3
float Mode3Factor = 0;           // How much to increase/decrease the mode 3 target temperature
byte UserTemp1 = 0;              // User selected mode 2 temperature or mode 3 start temperature
byte UserTemp2 = 0;              // User selected ending temperature in mode 3
byte UserTime = 0;               // User selected distillation run time in mode 3 (hours)
byte UserPower = 0;              // User selected power level in mode 1 (0% to 100%)
byte CurrentMode = 1;            // 1 = Constant Power, 2 = Constant Temp, 3 = Timed w/Temps
byte Mode3Direction = 1;         // Mode 3 temperature direction, 0 = decrease, 1 = increase
byte PowerLevel = 0;             // Current power level 0-255, (100/255) * PowerLevel = % Power
byte ActiveButton = 0;           // Currently selected touch-screen button
char Runtime[10];                // HH:MM:SS formatted time of the current distillation run
//------------------------------------------------------------------------------------------------
// Coordinates for touch-screen buttons (Modes 1 and 2)
int ModeX1 = 0, ModeY1 = 0, ModeX2 = 158, ModeY2 = 84;
int RunX1 = 160, RunY1 = 0, RunX2 = 319, RunY2 = 84;
int TempX1 = 0, TempY1 = 86, TempX2 = 158, TempY2 = 169;
int PowerX1 = 160, PowerY1 = 86, PowerX2 = 319, PowerY2 = 169;
// Coordinates for Mode 3 bottom row buttons
int StartX1 = 0, StartY1 = 0, StartX2 = 0, StartY2 = 0;
int EndX1 = 0, EndY1 = 0, EndX2 = 0, EndY2 = 0;
int TimeX1 = 0, TimeY1 = 0, TimeX2 = 0, TimeY2 = 0;
//------------------------------------------------------------------------------------------------
Arduino_DataBus *bus = new Arduino_ESP32PAR8Q(7 /* DC */, 6 /* CS */, 8 /* WR */, 9 /* RD */,39 /* D0 */, 40 /* D1 */, 41 /* D2 */, 42 /* D3 */, 45 /* D4 */, 46 /* D5 */, 47 /* D6 */, 48 /* D7 */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 5 /* RST */, 0 /* rotation */, true /* IPS */, 170 /* width */, 320 /* height */, 35 /* col offset 1 */, 0 /* row offset 1 */, 35 /* col offset 2 */, 0 /* row offset 2 */);
Arduino_Canvas_Indexed *canvas = new Arduino_Canvas_Indexed(320 /* width */, 170 /* height */, gfx);
TouchLib Touch(Wire,SDA,SCL,CTS820_SLAVE_ADDRESS,TOUCH_RES);
//------------------------------------------------------------------------------------------------
OneWire oneWire(ONE_WIRE);
DallasTemperature DT(&oneWire);
Preferences preferences;
//------------------------------------------------------------------------------------------------
void setup() {
  // Enable serial communications for debugging output
  Serial.begin(9600);
  while (! Serial) delay(10);
  Serial.println("");

  // Get the last user settings from flash memory
  GetMemory();
  if (UserTemp1 == 0) {
    // New chip, flash memory not initialized
    UserTemp1 = 80;
    UserTemp2 = 90;
    UserTime  = 4;
    UserPower = 80;
    SetMemory();
  }

  // Initialize the touch-screen reset line
  pinMode(TOUCH_RES,OUTPUT);
  digitalWrite(TOUCH_RES,LOW);
  delay(500);
  digitalWrite(TOUCH_RES,HIGH);

  // Initialize all of the necessary GPIO libraries
  DT.begin();
  Wire.begin(SDA,SCL);
  if (Touch.init()) {
    Serial.println("Touch screen interface initialized");
    attachInterrupt(TOUCH_INT,[] { GotInterrupt = true; },FALLING);
  } else {
    Serial.println("Touch screen interface not detected");
  }

  // Power up the screen and backlight
  pinMode(SCREEN_POWER_ON,OUTPUT);
  digitalWrite(SCREEN_POWER_ON,HIGH);
  ledcSetup(0,2000,8);
  ledcAttachPin(SCREEN_BACKLIGHT,0);
  ledcWrite(0,255); // Screen brightness (0-255)

  // Initialize the graphics library and blank the screen
  gfx->begin();
  gfx->setRotation(3);
  gfx->fillScreen(BLACK);

  // In order to eliminate screen flicker, everything is plotted to an off-screen buffer and popped to the screen when done
  canvas->begin();
  ScreenUpdate();

  // Assign the SCR controller output pin to a PWM channel
  ledcSetup(1,60,8); // 60 Hz AC power frequency, modify as necessary if your power is not 60 Hz
  ledcAttachPin(SCR_OUT,1);
  ledcWrite(1,0);

  // Initialize the timing related variables
  LoopCounter = millis();
  LastAdjustment = LoopCounter;
}
//------------------------------------------------------------------------------------------------
void GetMemory() { // Get the last user settings from flash memory on startup
  preferences.begin("prefs",true);
  UserTemp1 = preferences.getUInt("UserTemp1",0);
  UserTemp2 = preferences.getUInt("UserTemp2",0);
  UserTime  = preferences.getUInt("UserTime",0);
  UserPower = preferences.getUInt("UserPower",0);
  preferences.end();
}
//------------------------------------------------------------------------------------------------
void SetMemory() { // Update flash memory with the current user settings
  preferences.begin("prefs",false);
  preferences.putUInt("UserTemp1",UserTemp1);
  preferences.putUInt("UserTemp2",UserTemp2);
  preferences.putUInt("UserTime",UserTime);
  preferences.putUInt("UserPower",UserPower);
  preferences.end();
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
  ledcWrite(1,0);
  delay(1000);
  // Then progressively adjust the power level up to the requested percentage
  if (Percent > 0) {
    PowerLevel = round(Percent * 2.55);
    float x = 2.55;
    while (x <= PowerLevel) {
      ledcWrite(1,x);
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
      PowerAdjust(UserPower);
    }
  } else {
    ActiveRun = false;
    PowerAdjust(0);
  }
}
//-----------------------------------------------------------------------------------------------
void DrawButton(byte WhichOne) { // Draws the specified button on the screen
  canvas->setFont(&FreeSans10pt7b);
  canvas->setTextColor(WHITE);
  if (WhichOne == 0) {
    canvas->fillRoundRect(ModeX1,ModeY1,ModeX2 - ModeX1,ModeY2 - ModeY1,5,BLUE);
    if (ActiveButton == 0) canvas->drawRoundRect(ModeX1,ModeY1,ModeX2 - ModeX1,ModeY2 - ModeY1,5,WHITE);
    if (ActiveRun) {

    } else {

    }
  } else if (WhichOne == 1) {
    if (ActiveRun) {
      canvas->fillRoundRect(RunX1,RunY1,RunX2 - RunX1,RunY2 - RunY1,5,RED);
    } else {
      canvas->fillRoundRect(RunX1,RunY1,RunX2 - RunX1,RunY2 - RunY1,5,GREEN);
    }
    if (ActiveButton == 1) canvas->drawRoundRect(RunX1,RunY1,RunX2 - RunX1,RunY2 - RunY1,5,WHITE);
  } else if (WhichOne == 2) {
    canvas->fillRoundRect(TempX1,TempY1,TempX2 - TempX1,TempY2 - TempY1,5,MAGENTA);
    if (ActiveButton == 2) canvas->drawRoundRect(TempX1,TempY1,TempX2 - TempX1,TempY2 - TempY1,5,WHITE);
  } else if (WhichOne == 3) {
    canvas->fillRoundRect(PowerX1,PowerY1,PowerX2 - PowerX1,PowerY2 - PowerY1,5,YELLOW);
    if (ActiveButton == 2) canvas->drawRoundRect(PowerX1,PowerY1,PowerX2 - PowerX1,PowerY2 - PowerY1,5,WHITE);
  }
}
//-----------------------------------------------------------------------------------------------
void ScreenUpdate() { // Update button labels and highlight the active button
  canvas->fillScreen(BLACK);
  DrawButton(0);
  DrawButton(1);
  if (CurrentMode == 3) {
    DrawButton(4);
    DrawButton(5);
    DrawButton(6);
  } else {
    DrawButton(2);
    DrawButton(3);
  }
  canvas->flush();
}
//-----------------------------------------------------------------------------------------------
bool RegionPressed(int Xpos,int Ypos,int X1,int Y1,int X2,int Y2) { // Screen button press evaluator
  if ((Xpos >= X1) && (Xpos <= X2) && (Ypos >= Y1) && (Ypos <= Y2)) {
    return true;
  } else {
    return false;
  }
}
//-----------------------------------------------------------------------------------------------
void ProcessTouch(int Xpos,int Ypos) { // Handle touch-screen presses
  if (RegionPressed(Xpos,Ypos,ModeX1,ModeY1,ModeX2,ModeY2)) {
    // Mode button
    if (! ActiveRun) ActiveButton = 0;
  } else if (RegionPressed(Xpos,Ypos,RunX1,RunY1,RunX2,RunY2)) {
    // Start/Stop button
    ActiveButton = 1;
    if (ActiveRun) {
      RunState(0);
    } else {
      RunState(1);
    }
  }
  if (! ActiveRun) {
    if (CurrentMode == 3) {
      if (RegionPressed(Xpos,Ypos,StartX1,StartY1,StartX2,StartY2)) {
        // Start Temp button
        ActiveButton = 4;
      } else if (RegionPressed(Xpos,Ypos,EndX1,EndY1,EndX2,EndY2)) {
        // End Temp button
        ActiveButton = 5;
      } else if (RegionPressed(Xpos,Ypos,TimeX1,TimeY1,TimeX2,TimeY2)) {
        // Time button
        ActiveButton = 6;
      }
    } else {
      if (RegionPressed(Xpos,Ypos,TempX1,TempY1,TempX2,TempY2)) {
        // Temperature button
        if (CurrentMode == 2) ActiveButton = 2;
      } else if (RegionPressed(Xpos,Ypos,PowerX1,PowerY1,PowerX2,PowerY2)) {
        // Power button
        if (CurrentMode == 1) ActiveButton = 3;
      }
    }
  }
  DrawButton(ActiveButton);
}
//-----------------------------------------------------------------------------------------------
void IncValue(byte WhichOne) { // Increment the value associated with the active screen button
  if (WhichOne == 0) {
    if (CurrentMode < 3) CurrentMode ++;
  } else if (WhichOne == 2) {
    if (UserTemp1 < 100) UserTemp1 ++;
  } else if (WhichOne == 3) {
    if (UserPower < 100) UserPower ++;
  } else if (WhichOne == 4) {
    if (UserTemp1 < 100) UserTemp1 ++;
  } else if (WhichOne == 5) {
    if (UserTemp2 < 100) UserTemp2 ++;
  } else if (WhichOne == 6) {
    if (UserTime < 24) UserTime ++;
  }
  DrawButton(WhichOne);
}
//-----------------------------------------------------------------------------------------------
void DecValue(byte WhichOne) { // Decrement the value associated with the active screen button
  if (WhichOne == 0) {
    if (CurrentMode > 1) CurrentMode --;
  } else if (WhichOne == 2) {
    if (UserTemp1 > 30) UserTemp1 --;
  } else if (WhichOne == 3) {
    if (UserPower > 1) UserPower --;
  } else if (WhichOne == 4) {
    if (UserTemp1 > 30) UserTemp1 --;
  } else if (WhichOne == 5) {
    if (UserTemp2 > 30) UserTemp2 --;
  } else if (WhichOne == 6) {
    if (UserTime > 1) UserTime --;
  }
  DrawButton(WhichOne);
}
//-----------------------------------------------------------------------------------------------
void ProcessButton(byte WhichOne) { // Handle increment/decrement button inputs
  byte HoldCounter = 0;

  if (WhichOne == 1) {
    // Increment active screen button value by 1
    IncValue(ActiveButton);
    while (digitalRead(INC_BTN) == 0) {
      delay(10);
      HoldCounter ++;
      if (HoldCounter == 200) { // User is intentionally holding the + button
        while (digitalRead(INC_BTN) == 0) {
          IncValue(ActiveButton);
          delay(500);
        }
      }
    }
  } else {
    // Decrement active screen button value by 1
    DecValue(ActiveButton);
    while (digitalRead(DEC_BTN) == 0) {
      delay(10);
      HoldCounter ++;
      if (HoldCounter == 200) { // User is intentionally holding the - button
        while (digitalRead(DEC_BTN) == 0) {
          DecValue(ActiveButton);
          delay(500);
        }
      }
    }
  }
  SetMemory();
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
  if (GotInterrupt) {
    if (Touch.read()) {
      TP_Point Point = Touch.getPoint(0);
      ProcessTouch(Point.x,Point.y);
      while (Touch.read()) delay(10);
    }
    GotInterrupt = false;
  }
  // Check for Value+ keypresses and handle as necessary
  if ((digitalRead(INC_BTN) == 0) && (! ActiveRun)) ProcessButton(1);
  // Check for Value- keypresses and handle as necessary
  if ((digitalRead(DEC_BTN) == 0) && (! ActiveRun)) ProcessButton(0);
  // Perform status updates every second without using a delay(1000) in the loop
  // The delay function can't be used in this loop due to the buttons and touch-screen
  if (CurrentTime - LoopCounter >= 1000) {
    Serial.println("Running Status Updates");
    Serial.print("Current Mode: "); Serial.println(CurrentMode);
    TempUpdate(); // Read the DS18B20 temperature
    // Safety net in case of thermal runaway
    if (TempC > 103) {
      Serial.println("Shutdown due to thermal runaway");
      RunState(0);
    }
    // Handle the active distillation run
    if (ActiveRun) {
      unsigned long allSeconds = (CurrentTime - StartTime) / 1000;
      int runHours = allSeconds / 3600;
      int secsRemaining = allSeconds % 3600;
      int runMinutes = secsRemaining / 60;
      int runSeconds = secsRemaining % 60;
      sprintf(Runtime,"%02u:%02u:%02u",runHours,runMinutes,runSeconds);
      Serial.print("Run Time: "); Serial.println(Runtime);
      if (CurrentMode > 1) { // Mode 1 is constant power, no temperature management
        if (! UpToTemp) {
          if (TempC >= UserTemp1) { // Minimum operating temperature has been reached
            UpToTemp = true;
            PowerAdjust(70); // Fall back to 70% power and begin temperature management
          }
        } else {
          if (CurrentMode == 2) { // Constant temperature
            Serial.print("Target Temp: "); Serial.println(UserTemp1,2);
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
            Serial.print("Power Percent: "); Serial.println(CurrentPercent);
          } else { // Timed distillation run with progressive temperature adjustment
            if (CurrentTime - StartTime < UserTime) {
              if (CurrentTime - Mode3Counter >= 900000) { // Adjust the target temperature every 15 minutes
                if (Mode3Direction == 1) {
                  Mode3Temp += Mode3Factor; // Increase the target temperature
                } else {
                  Mode3Temp -= Mode3Factor; // Decrease the target temperature
                }
                Mode3Counter = CurrentTime;
              }
              Serial.print("Target Temp: "); Serial.println(Mode3Temp,2);
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
              Serial.print("Power Percent: "); Serial.println(CurrentPercent);
            } else { // Distillation run time has expired, shut down
              RunState(0);
            }
          }
        }
      }
    }
    ScreenUpdate();
    LoopCounter = CurrentTime;
  }
}
//------------------------------------------------------------------------------------------------
