//------------------------------------------------------------------------------------------------
// Airhead - Air Still Brain | (CopyLeft) 2024-Present | Larry Athey (https://panhandleponics.com)
//
// Tested up to Espressif ESP32 v2.0.14 due to LilyGo requirements
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
// No, I do not use anybody else's user interface library for the user interface here. They're all
// entirely too bloated for what I need to do. Since I wrote my own GUI for DOS BBS software back
// in the 1990s (MAX Graphics) it wasn't really all that complicated for me to do it again on this
// little computer. It's not flashy, but it's also not unnecessarily bogged down with junk code.
//
// My suggestion for the installation of the temperature sensor is to use one in a TO-92 case and
// attach it to the bolt near the front of the boiler vessel using a transistor chassis mount or
// a copper wiring lug crimped down tight against the temperature sensor. Use wire with silicone
// insulation for all connections and heat shrink the leads to prevent any shorts. You absolutely
// need good thermal transfer compound at all contact points, even around the sensor itself!
//
// This controller will work with any SCR board that accepts a PWM input. If your SCR board uses
// variable voltage, you would just feed a PWM to voltage convertor module between this and your
// your SCR board. Or, if you comment out the SCR_OUT constant definition, you can use this with
// any zero-crossing trigger solid state relay. Absolutely do not use a random-turn-on SSR! SCR
// board or a solid state relay "Boilermaker style", the choice is totally up to you.
//
// You will see that this code has two app modes, Airhead and μBoilermaker. This is toggled from
// one mode to the other by selecting the Mode button on the screen and holding the Value+ button
// for 30 seconds. μBoilermaker uses PID temperature management rather than the PI method used by
// the Airhead mode. This mode is intended for use in a special adaptation of this project built
// into 1800 watt cast iron electric hot plates, commonly used with stove-top stills & brew pots.
//
// NOTE: You can still use a solid state relay in the place of an SCR board, but at a much
//       lower frequency. If your mains power is 50 Hz, then you would use 100 Hz for your
//       PWM frequency. If your mains power is 60 Hz, you would use 120 Hz instead. Please
//       keep in mind that this will cause the SSR to generate more heat, use a heat sink!
//------------------------------------------------------------------------------------------------
#include "Arduino_GFX_Library.h" // Standard GFX library for Arduino, built with version 1.5.9
#include "FreeSans9pt7b.h"       // https://github.com/moononournation/ArduinoFreeFontFile.git 
#include "OneWire.h"             // OneWire Network communications library
#include "DallasTemperature.h"   // Dallas Semiconductor DS18B20 temperature sensor library
#include "Preferences.h"         // ESP32 Flash memory read/write library
#include "Wire.h"                // I2C communications library for touch-screen interface
#define TOUCH_MODULES_CST_SELF   // Tell TouchLib.h to use the CST816 chip routines
#include "TouchLib.h"            // LilyGo touch-screen interface library
#include "ota_update.h"          // Over-The-Air firmware updating library
#include "QuickPID.h"            // PID calculation library from https://github.com/Dlloydev/QuickPID (used in μBoilermaker mode)
//------------------------------------------------------------------------------------------------
#define ONE_WIRE 13              // 1-Wire network pin for the DS18B20 temperature sensor
//#define SCR_OUT 1              // PWM output to an SCR board (comment out if using a Boilermaker style SSR)
#define FAN_OUT 2                // Cooling fan on/off toggle (optional)
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
bool ConfigMode = false;         // True if the configuration mode is active
bool GotInterrupt = false;       // True if touch input has been detected on the screen
bool UpToTemp = false;           // True if the run startup has reached operating temperature
bool UpdateMode = false;         // True if the Airhead is running in firmware update mode
unsigned long StartTime = 0;     // Start time of the current distillation run
unsigned long FallBackTime = 0;  // Time of the still finally reached minimum operating temperature
unsigned long LoopCounter = 0;   // Timekeeper for the loop to eliminate the need to delay it
unsigned long LastAdjustment = 0;// Time of the last power adjustment
long Mode3Counter = 0;           // 15 minute timer for mode 3 target temperature adjustments
float TempC = 0;                 // Current temperature reading C
float TempF = 0;                 // Current temperature reading F
float Mode3Temp = 0;             // Current target temperature when running in mode 3
float Mode3Factor = 0;           // How much to increase/decrease the mode 3 target temperature
float Correction = 0.0;          // How much to correct temp sensor C readings (positive or negative)
byte UserTemp1 = 0;              // User selected mode 2 temperature or mode 3 start temperature
byte UserTemp2 = 0;              // User selected ending temperature in mode 3
byte UserTime = 0;               // User selected distillation run time in mode 3 (hours)
byte UserPower = 0;              // User selected power level in mode 1 (0% to 100%)
byte UserMode = 1;               // Last used operation mode selected by the user
byte CurrentMode = 1;            // 1 = Constant Power, 2 = Constant Temp, 3 = Timed w/Temps
byte Mode3Direction = 1;         // Mode 3 temperature direction, 0 = decrease, 1 = increase
byte PowerLevel = 0;             // Current power level 0-255, (100/255) * PowerLevel = % Power
byte StartupPower = 0;           // Startup power percentage in temperature managed modes
byte FallbackPower = 0;          // Fallback power percentage in temperature managed modes
byte RestPeriod = 0;             // How many seconds to rest after the target temperature is reached
byte ActiveButton = 0;           // Currently selected touch-screen button
byte AppMode = 0;                // 0 = Airhead mode, 1 = μBoilermaker mode
char Runtime[10];                // HH:MM:SS formatted time of the current distillation run
String Version = "1.0.2b";       // Current release version of the project
//------------------------------------------------------------------------------------------------
// Coordinates for touch-screen buttons (Modes 1 and 2)
int ModeX1 = 0, ModeY1 = 0, ModeX2 = 158, ModeY2 = 84;
int RunX1 = 160, RunY1 = 0, RunX2 = 319, RunY2 = 84;
int PowerX1 = 0, PowerY1 = 86, PowerX2 = 158, PowerY2 = 169;
int TempX1 = 160, TempY1 = 86, TempX2 = 319, TempY2 = 169;
// Coordinates for Mode 3 bottom row buttons
int StartX1 = 0, StartY1 = 86, StartX2 = 105, StartY2 = 169;
int EndX1 = 107, EndY1 = 86, EndX2 = 213, EndY2 = 169;
int TimeX1 = 215, TimeY1 = 86, TimeX2 = 319, TimeY2 = 169;
// RGB values for the button and text colors
#define MODEBTN RGB565(200,0,200)
#define RUNBTN RGB565(0,215,0)
#define STOPBTN RGB565(230,0,0)
#define PWRBTN RGB565(0,210,210)
#define TEMPBTN RGB565(50,50,230)
#define STARTBTN RGB565(0,210,210)
#define ENDBTN RGB565(0,210,210)
#define TIMEBTN RGB565(50,50,230)
#define HILITE RGB565(230,230,230)
#define BTNTEXT RGB565(245,245,245)
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
// v1.0.2b add-on to provide PID control in μBoilermaker app mode
float targetTemp = 32.0;         // PID target temperature
float pidOutput = 0.0;           // PID Computed PWM percentage (0-100)
float Kp = 1.0;                  // PID Proportional gain (0.1 to 10.0)
float Ki = 0.005;                // PID Integral gain (0.001 to 0.5)
float Kd = 1.0;                  // PID Derivative gain (0.0 to 2.0)
float sampleTime = 10.0;         // PID Sample time (5 to 30 seconds)
QuickPID myPID(&TempC,&pidOutput,&targetTemp,Kp,Ki,Kd,
               QuickPID::pMode::pOnMeas,
               QuickPID::dMode::dOnMeas,
               QuickPID::iAwMode::iAwCondition,
               QuickPID::Action::direct);
//------------------------------------------------------------------------------------------------
#ifndef SCR_OUT
#include "esp_timer.h"   // High resolution timer library for use with interrupt driven code
#include "driver/gpio.h" // ESP-IDF GPIO library
//------------------------------------------------------------------------------------------------
#define SSR_OUT GPIO_NUM_1 // Same pin as used with an SCR board

int dutyCyclePercentage = 0;
hw_timer_t *timer = NULL;
//------------------------------------------------------------------------------------------------
void IRAM_ATTR onTimer() { // Custom low frequency PWM designed specifically for SSR usage
  static uint32_t cycleCounter = 0;
  cycleCounter ++;
    
  if (cycleCounter == 10) { // Reset counter every 10 interrupts (equivalent to 2.5 seconds if interrupt every 250ms)
    cycleCounter = 0;
  }

  if (cycleCounter < (dutyCyclePercentage / 10.0)) { // Turn on if within duty cycle
    gpio_set_level(SSR_OUT,1);
  } else {
    gpio_set_level(SSR_OUT,0);
  }
}
#endif
//------------------------------------------------------------------------------------------------
void setup() {
  // Enable serial communications for debugging output
  Serial.begin(9600);
  delay(1000);
  Serial.println("");

  // Get the last user settings from flash memory
  GetMemory();
  if (UserTemp1 == 0) {
    // System reset initiated or this is a new chip, initialize the flash memory
    UserTemp1     = 80;
    UserTemp2     = 86;
    UserTime      = 2;
    UserPower     = 80;
    UserMode      = 1;
    Correction    = 0.0;
    StartupPower  = 50;
    FallbackPower = 33;
    RestPeriod    = 60;
    Kp            = 1.0;
    Ki            = 0.005;
    Kd            = 1.0;
    SetMemory();
  }
  CurrentMode = UserMode;

  // Initialize the touch-screen reset line
  pinMode(TOUCH_RES,OUTPUT);
  digitalWrite(TOUCH_RES,LOW);
  delay(500);
  digitalWrite(TOUCH_RES,HIGH);

  // Initialize the optional fan toggle pin
  pinMode(FAN_OUT,OUTPUT);
  digitalWrite(FAN_OUT,LOW);

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

  // In order to eliminate screen flicker, everything is plotted to an off-screen buffer and popped onto the screen when done
  canvas->begin();
  ScreenUpdate();
  if (AppMode == 0) {
    PopoverMessage("Airhead v" + Version);
  } else {
    PopoverMessage("Micro Boilermaker v" + Version);
    myPID.SetOutputLimits(0,100);
    myPID.SetSampleTimeUs(sampleTime * 1000000);
    myPID.SetMode(myPID.Control::automatic);
  }
  delay(2500);
  ScreenUpdate();

  #ifndef SCR_OUT
  gpio_set_direction(SSR_OUT,GPIO_MODE_OUTPUT);
  gpio_set_level(SSR_OUT,0);
  // Air Still heating elements have a slow reaction time, an SCR's switching frequency is wasteful
  // Therefore, a custom low frequency PWM driving a solid state relay is a better option
  timer = timerBegin(0,80,true); // Timer at 1 MHz, count up
  timerAttachInterrupt(timer,&onTimer,true); // Attach the PWM toggle function
  timerAlarmWrite(timer,250000,true); // Timer trigger set to 250ms (250,000 microseconds)
  timerAlarmEnable(timer); // Now enable the low frequency pulse width modulator
  #else
  // Assign the SCR controller output pin to a PWM channel
  // For heating elements, 1 KHz to 3 KHz is used, adjust as necessary
  ledcSetup(1,2000,8);
  ledcAttachPin(SCR_OUT,1);
  ledcWrite(1,0);
  #endif

  // Initialize the timing related variables
  LoopCounter = millis();
  LastAdjustment = LoopCounter;

  // If the Value- is being pressed, activate the firmware update mode
  if (digitalRead(DEC_BTN) == 0) {
    UpdateMode = true;
    PopoverMessage("Firmware Update Mode");
    Serial.println("Starting Airhead Firmware Updater (AP mode)...");

    // Start WiFi Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid,ap_password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    // Start Bonjour/ZeroConf
    if (MDNS.begin("esp32")) {
      Serial.println("mDNS started - http://esp32.local");
    }

    // Set the server home page, sent upon browser connection
    server.on("/",HTTP_GET,[]() {
      server.send(200,"text/html",serverIndex);
    });

    // Set the OTA firmware update handler
    server.on("/update",HTTP_POST,[]() {
      server.sendHeader("Connection","close");
      server.send(200,"text/plain",(Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    },[]() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n",upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf,upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          Serial.printf("Update Success: %u bytes\n",upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      }
    });

    server.begin();
    Serial.println("HTTP server ready. Connect to the AP and go to 192.168.4.1");
  }
}
//------------------------------------------------------------------------------------------------
void GetMemory() { // Get the last user settings from flash memory on startup
  preferences.begin("prefs",true);
  UserTemp1     = preferences.getUInt("usertemp1",0);
  UserTemp2     = preferences.getUInt("usertemp2",0);
  UserTime      = preferences.getUInt("usertime",0);
  UserPower     = preferences.getUInt("userpower",0);
  UserMode      = preferences.getUInt("usermode",0);
  Correction    = preferences.getFloat("correction",0.0);
  StartupPower  = preferences.getUInt("startuppower",50);
  FallbackPower = preferences.getUInt("fallbackpower",33);
  RestPeriod    = preferences.getUInt("restperiod",60);
  AppMode       = preferences.getUInt("appmode",0);
  Kp            = preferences.getFloat("pid_kp",1.0);
  Ki            = preferences.getFloat("pid_ki",0.005);
  Kd            = preferences.getFloat("pid_kd",1.0);
  preferences.end();
}
//------------------------------------------------------------------------------------------------
void SetMemory() { // Update flash memory with the current user settings
  preferences.begin("prefs",false);
  preferences.putUInt("usertemp1",UserTemp1);
  preferences.putUInt("usertemp2",UserTemp2);
  preferences.putUInt("usertime",UserTime);
  preferences.putUInt("userpower",UserPower);
  preferences.putUInt("usermode",UserMode);
  preferences.putFloat("correction",Correction);
  preferences.putUInt("startuppower",StartupPower);
  preferences.putUInt("fallbackpower",FallbackPower);
  preferences.putUInt("restperiod",RestPeriod);
  preferences.putUInt("appmode",AppMode);
  preferences.putFloat("pid_kp",Kp);
  preferences.putFloat("pid_ki",Ki);
  preferences.putFloat("pid_kd",Kd);
  preferences.end();
}
//------------------------------------------------------------------------------------------------
void TempUpdate() { // Update the temperature sensor values
  DT.requestTemperatures();
  float Test = DT.getTempCByIndex(0); // Returns -127.00 if the device reading fails
  if (Test > -127.00) {
    TempC = Test;
    TempC += Correction;
  }
  TempF = TempC * 9 / 5 + 32;
  Serial.print("Temp C: "); Serial.println(TempC);
  Serial.print("Temp F: "); Serial.println(TempF);
}
//-----------------------------------------------------------------------------------------------
void PowerAdjust(byte Percent) { // Set the SCR board or SSR timing to a target power percentage
  Serial.print("Power Adjust: "); Serial.println(Percent);
  LastAdjustment = millis();
  #ifndef SCR_OUT
  // This is a digital power controller, there are only on and off states
  dutyCyclePercentage = Percent;
  PowerLevel = round(Percent * 2.55);
  #else
  // This is an analog power controller, first set the power level to zero
  // and rest 1 second so all of the capacitors can fully discharge
  ledcWrite(1,0);
  delay(1000);
  // Then progressively adjust the power level up to the requested percentage
  if (Percent > 0) {
    PowerLevel = round(Percent * 2.55);
    float x = 2.55;
    while (x <= PowerLevel) {
      ledcWrite(1,x); // Function appears to round x on its own, no errors thrown
      delay(10);
      x += 2.55;
    }
  } else {
    PowerLevel = 0;
  }
  #endif
}
//-----------------------------------------------------------------------------------------------
void RunState(byte State) { // Toggle the active distillation run state
  if (State == 1) {
    StartTime = millis();
    ActiveRun = true;
    UpToTemp  = false;
    digitalWrite(FAN_OUT,HIGH);
    if (CurrentMode > 1) {
      if (CurrentMode == 3) {
        byte Range;
        if (UserTemp1 < UserTemp2) {
          Mode3Direction = 1;
          Mode3Temp = float(UserTemp1);
          Range = UserTemp2 - UserTemp1;
        } else {
          Mode3Direction = 0;
          Mode3Temp = float(UserTemp2);
          Range = UserTemp1 - UserTemp2;
        }
        Mode3Factor  = float(Range) / float(UserTime * 4);
        Mode3Counter = millis();
      }
      if (AppMode == 1) {
        if (CurrentMode == 3) {
          targetTemp = Mode3Temp;
        } else {
          targetTemp = float(UserTemp1);
        }
        myPID.Reset();
        myPID.SetTunings(Kp,Ki,Kd);
        myPID.SetSampleTimeUs(sampleTime * 1000000);
      } else {
        PowerAdjust(StartupPower);
      }
    } else {
      PowerAdjust(UserPower);
    }
  } else {
    ActiveRun = false;
    PowerAdjust(0);
    digitalWrite(FAN_OUT,LOW);
  }
}
//-----------------------------------------------------------------------------------------------
void PopoverMessage(String Msg) { // Display popover message to the user
  int16_t nX = 0, nY = 0, TextX;
  uint16_t nWidth = 0, nHeight = 0;

  canvas->setFont(&FreeSans9pt7b);
  canvas->setTextColor(BLACK);
  canvas->getTextBounds(Msg,0,0,&nX,&nY,&nWidth,&nHeight);
  TextX = round(nWidth / 2);
  canvas->fillRoundRect(160 - TextX - 12,60,nWidth + 26,40,5,WHITE);
  canvas->drawRoundRect(160 - TextX - 12,60,nWidth + 26,40,5,BLACK);
  canvas->setCursor(160 - TextX,85);
  canvas->print(Msg);
  canvas->flush();
}
//------------------------------------------------------------------------------------------------
void DrawButton(byte WhichOne) { // Draws and highlights the specified button on the screen
  int CurrentPercent = round(0.392156863 * PowerLevel);
  byte Ftemp;

  canvas->setFont(&FreeSans9pt7b);
  canvas->setTextColor(BTNTEXT);
  if (WhichOne == 0) {
    if (ConfigMode) {
      canvas->fillRoundRect(ModeX1,ModeY1,ModeX2 - ModeX1,ModeY2 - ModeY1,5,DARKGREY);
      if (ActiveButton == 0) canvas->drawRoundRect(ModeX1,ModeY1,ModeX2 - ModeX1,ModeY2 - ModeY1,5,HILITE);
      canvas->setCursor(ModeX1 + 35,ModeY1 + 25);
      canvas->print("Correction");
      canvas->setCursor(ModeX1 + 52,ModeY1 + 45);
      canvas->print("Factor");
      canvas->setCursor(ModeX1 + 57,ModeY1 + 65);
      canvas->printf("%2.1fC",Correction);
    } else {
      canvas->fillRoundRect(ModeX1,ModeY1,ModeX2 - ModeX1,ModeY2 - ModeY1,5,MODEBTN);
      if (ActiveButton == 0) canvas->drawRoundRect(ModeX1,ModeY1,ModeX2 - ModeX1,ModeY2 - ModeY1,5,HILITE);
       if (ActiveRun) {
        // Run Time
        canvas->setCursor(ModeX1 + 10,ModeY1 + 25);
        canvas->printf("R: %2s",Runtime);
        // Power Level
        canvas->setCursor(ModeX1 + 10,ModeY1 + 45);
        canvas->printf("P: %2u%%",CurrentPercent);
        // Temperature
        canvas->setCursor(ModeX1 + 10,ModeY1 + 65);
        canvas->printf("T: %2.1fC / %2.1fF",TempC,TempF);
      } else {
        canvas->setCursor(ModeX1 + 45,ModeY1 + 35);
        canvas->printf("Mode %2u",CurrentMode);
        canvas->setCursor(ModeX1 + 15,ModeY1 + 55);
        if (CurrentMode == 1) {
          canvas->print("Constant Power");
        } else if (CurrentMode == 2) {
          canvas->print("Constant Temp");    
        } else if (CurrentMode == 3) {
          canvas->print("Timed w/Temp"); 
        }
      }
    }
  } else if (WhichOne == 1) {
    if (ConfigMode) {
      canvas->fillRoundRect(RunX1,RunY1,RunX2 - RunX1,RunY2 - RunY1,5,STOPBTN);
      canvas->setCursor(RunX1 + 35,RunY1 + 35);
      canvas->print("Restore All");
      canvas->setCursor(RunX1 + 13,RunY1 + 55);
      canvas->print("System Defaults");
    } else {
      if (ActiveRun) {
        canvas->fillRoundRect(RunX1,RunY1,RunX2 - RunX1,RunY2 - RunY1,5,STOPBTN);
        canvas->setCursor(RunX1 + 60,RunY1 + 35);
        canvas->print("Stop");
      } else {
        canvas->fillRoundRect(RunX1,RunY1,RunX2 - RunX1,RunY2 - RunY1,5,RUNBTN);
        canvas->setCursor(RunX1 + 58,RunY1 + 35);
        canvas->print("Start");
      }
      canvas->setCursor(RunX1 + 18,RunY1 + 55);
      canvas->print("Distillation Run");
    }
    if (ActiveButton == 1) canvas->drawRoundRect(RunX1,RunY1,RunX2 - RunX1,RunY2 - RunY1,5,HILITE);
  } else if (WhichOne == 2) {
    canvas->fillRoundRect(PowerX1,PowerY1,PowerX2 - PowerX1,PowerY2 - PowerY1,5,PWRBTN);
    canvas->setCursor(PowerX1 + 26,PowerY1 + 35);
    canvas->print("Power Level");
    if (CurrentMode == 1) {
      canvas->setCursor(PowerX1 + 58,PowerY1 + 55);
      canvas->printf("%2u%%",UserPower);
    } else {
      canvas->setCursor(PowerX1 + 34,PowerY1 + 55);
      canvas->print("[Managed]");
    }
    if (ActiveButton == 2) canvas->drawRoundRect(PowerX1,PowerY1,PowerX2 - PowerX1,PowerY2 - PowerY1,5,HILITE); 
  } else if (WhichOne == 3) {
    Ftemp = round(UserTemp1 * 9 / 5 + 32);
    canvas->fillRoundRect(TempX1,TempY1,TempX2 - TempX1,TempY2 - TempY1,5,TEMPBTN);
    canvas->setCursor(TempX1 + 26,TempY1 + 35);
    
    canvas->print("Temperature");
    if (CurrentMode == 2) {
      canvas->setCursor(TempX1 + 35,TempY1 + 55);
      canvas->printf("%2uC / %2uF",UserTemp1,Ftemp);
    } else {
      canvas->setCursor(TempX1 + 20,TempY1 + 55);
      canvas->print("[Not Managed]");
    }
    if (ActiveButton == 3) canvas->drawRoundRect(TempX1,TempY1,TempX2 - TempX1,TempY2 - TempY1,5,HILITE);
  } else if (WhichOne == 4) {
    if (ConfigMode) {
      canvas->fillRoundRect(StartX1,StartY1,StartX2 - StartX1,StartY2 - StartY1,5,DARKGREY);
      if (AppMode == 0) {
        canvas->setCursor(StartX1 + 23,StartY1 + 25);
        canvas->print("Startup");
        canvas->setCursor(StartX1 + 28,StartY1 + 45);
        canvas->print("Power");
        canvas->setCursor(StartX1 + 36,StartY1 + 65);
        canvas->printf("%2u%%",StartupPower);
      } else {
        canvas->setCursor(StartX1 + 37,StartY1 + 25);
        canvas->print("PID");
        canvas->setCursor(StartX1 + 25,StartY1 + 45);
        canvas->print("P Gain");
        canvas->setCursor(StartX1 + 37,StartY1 + 65);
        canvas->printf("%.1f",Kp);  
      }
    } else {
      canvas->fillRoundRect(StartX1,StartY1,StartX2 - StartX1,StartY2 - StartY1,5,STARTBTN);
      canvas->setCursor(StartX1 + 33,StartY1 + 25);
      canvas->print("Start");
      canvas->setCursor(StartX1 + 33,StartY1 + 45);
      canvas->printf("%2uC",UserTemp1);
      Ftemp = round(UserTemp1 * 9 / 5 + 32);
      canvas->setCursor(StartX1 + 33,StartY1 + 65);
      canvas->printf("%2uF",Ftemp);
    }
    if (ActiveButton == 4) canvas->drawRoundRect(StartX1,StartY1,StartX2 - StartX1,StartY2 - StartY1,5,HILITE);
  } else if (WhichOne == 5) {
    if (ConfigMode) {
      canvas->fillRoundRect(EndX1,EndY1,EndX2 - EndX1,EndY2 - EndY1,5,DARKGREY);
      if (AppMode == 0) {
        canvas->setCursor(EndX1 + 20,EndY1 + 25);
        canvas->print("Fallback");
        canvas->setCursor(EndX1 + 28,EndY1 + 45);
        canvas->print("Power");
        canvas->setCursor(EndX1 + 36,EndY1 + 65);
        canvas->printf("%2u%%",FallbackPower);
      } else {
        canvas->setCursor(EndX1 + 37,EndY1 + 25);
        canvas->print("PID");
        canvas->setCursor(EndX1 + 27,EndY1 + 45);
        canvas->print("I Gain");
        canvas->setCursor(EndX1 + 30,EndY1 + 65);
        canvas->printf("%.3f",Ki);
      }
    } else {
      canvas->fillRoundRect(EndX1,EndY1,EndX2 - EndX1,EndY2 - EndY1,5,ENDBTN);
      canvas->setCursor(EndX1 + 36,EndY1 + 25);
      canvas->print("End");
      canvas->setCursor(EndX1 + 36,EndY1 + 45);
      canvas->printf("%2uC",UserTemp2);
      Ftemp = round(UserTemp2 * 9 / 5 + 32);
      canvas->setCursor(EndX1 + 36,EndY1 + 65);
      canvas->printf("%2uF",Ftemp);
    }
    if (ActiveButton == 5) canvas->drawRoundRect(EndX1,EndY1,EndX2 - EndX1,EndY2 - EndY1,5,HILITE);
  } else if (WhichOne == 6) {
    if (ConfigMode) {
      canvas->fillRoundRect(TimeX1,TimeY1,TimeX2 - TimeX1,TimeY2 - TimeY1,5,DARKGREY);
      if (AppMode == 0) {
        canvas->setCursor(TimeX1 + 32,TimeY1 + 25);
        canvas->print("Rest");
        canvas->setCursor(TimeX1 + 25,TimeY1 + 45);
        canvas->print("Period");
        canvas->setCursor(TimeX1 + 36,TimeY1 + 65);
        canvas->printf("%2us",RestPeriod);
      } else {
        canvas->setCursor(TimeX1 + 37,TimeY1 + 25);
        canvas->print("PID");
        canvas->setCursor(TimeX1 + 25,TimeY1 + 45);
        canvas->print("D Gain");
        canvas->setCursor(TimeX1 + 37,TimeY1 + 65);
        canvas->printf("%.1f",Kd);
      }
    } else {
      canvas->fillRoundRect(TimeX1,TimeY1,TimeX2 - TimeX1,TimeY2 - TimeY1,5,TIMEBTN);
      canvas->setCursor(TimeX1 + 30,TimeY1 + 25);
      canvas->print("Time");
      canvas->setCursor(TimeX1 + 40,TimeY1 + 45);
      canvas->printf("%2u",UserTime);
      canvas->setCursor(TimeX1 + 28,TimeY1 + 65);
      canvas->print("Hours");
    }
    if (ActiveButton == 6) canvas->drawRoundRect(TimeX1,TimeY1,TimeX2 - TimeX1,TimeY2 - TimeY1,5,HILITE);
  }
}
//-----------------------------------------------------------------------------------------------
void ScreenUpdate() { // Update button labels and highlight the active button
  canvas->fillScreen(BLACK);
  DrawButton(0);
  DrawButton(1);
  if ((CurrentMode == 3) || (ConfigMode)) {
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
  // Yes, the multiple ScreenUpdate() calls seems sloppy or careless.
  // I do this in order to provide quicker visual feedback to the user
  // because the loop() function only updates the screen every second.
  if ((! ActiveRun) && (Xpos < 0)) { // Home button pressed, toggle ConfigMode if the system is idle
    ConfigMode = !ConfigMode;
    if (!ConfigMode) {
      PopoverMessage("Writing to flash memory");
      delay(2500);
    }
    ActiveButton = 0;
    ScreenUpdate();
    return;
  }
  if (RegionPressed(Xpos,Ypos,ModeX1,ModeY1,ModeX2,ModeY2)) {
    // Mode and Correction Offset button
    if (! ActiveRun) {
      ActiveButton = 0;
      ScreenUpdate();
    }
  } else if (RegionPressed(Xpos,Ypos,RunX1,RunY1,RunX2,RunY2)) {
    // Start/Stop and System Reset button
    ActiveButton = 1;
    ScreenUpdate();
    if (ConfigMode) { // System reset requested
      UserTemp1 = 0;
      SetMemory();
      PopoverMessage("Restoring system defaults");
      delay(3000);
      ESP.restart();
    } else { // Toggle run state
      if (ActiveRun) {
        RunState(0);
      } else {
        RunState(1);
      }
      ScreenUpdate();
    }
  }
  if (! ActiveRun) {
    if ((CurrentMode == 3) || (ConfigMode)) {
      if (RegionPressed(Xpos,Ypos,StartX1,StartY1,StartX2,StartY2)) {
        // Start Temp and Startup Power button
        ActiveButton = 4;
      } else if (RegionPressed(Xpos,Ypos,EndX1,EndY1,EndX2,EndY2)) {
        // End Temp and Fallback Power button
        ActiveButton = 5;
      } else if (RegionPressed(Xpos,Ypos,TimeX1,TimeY1,TimeX2,TimeY2)) {
        // Run Time and Rest Period button
        ActiveButton = 6;
      }
    } else {
      if (RegionPressed(Xpos,Ypos,TempX1,TempY1,TempX2,TempY2)) {
        // Temperature button
        if (CurrentMode == 2) ActiveButton = 3;
      } else if (RegionPressed(Xpos,Ypos,PowerX1,PowerY1,PowerX2,PowerY2)) {
        // Power button
        if (CurrentMode == 1) ActiveButton = 2;
      }
    }
    ScreenUpdate();
  }
}
//-----------------------------------------------------------------------------------------------
void IncValue(byte WhichOne) { // Increment the value associated with the active screen button
  if (ConfigMode) {
    if (WhichOne == 0) {
      if (Correction < 15.0) Correction += .1;
    } else if (WhichOne == 2) {
      // Not used in config mode
    } else if (WhichOne == 3) {
      // Not used in config mode
    } else if (WhichOne == 4) {
      if (AppMode == 0) {
        if (StartupPower < 100) StartupPower ++;
      } else{
        if (Kp < 10.0) Kp += 0.1;
      }
    } else if (WhichOne == 5) {
      if (AppMode == 0) {
        if (FallbackPower < 100) FallbackPower ++;
      } else {
        if (Ki < 0.5) Ki += 0.001;
      }
    } else if (WhichOne == 6) {
      if (AppMode == 0) {
        if (RestPeriod < 255) RestPeriod ++;
      } else {
        if (Kd < 2.0) Kd += 0.1;
      }
    }
  } else {
    if (WhichOne == 0) {
      if (CurrentMode < 3) CurrentMode ++;
    } else if (WhichOne == 2) {
      if (UserPower < 100) UserPower ++;
    } else if (WhichOne == 3) {
      if (UserTemp1 < 100) UserTemp1 ++;
    } else if (WhichOne == 4) {
      if (UserTemp1 < 100) UserTemp1 ++;
    } else if (WhichOne == 5) {
      if (UserTemp2 < 100) UserTemp2 ++;
    } else if (WhichOne == 6) {
      if (UserTime < 24) UserTime ++;
    }
  }
  ScreenUpdate();
}
//-----------------------------------------------------------------------------------------------
void DecValue(byte WhichOne) { // Decrement the value associated with the active screen button
  if (ConfigMode) {
    if (WhichOne == 0) {
      if (Correction > -15.0) Correction -= .1;
    } else if (WhichOne == 2) {
      // Not used in config mode
    } else if (WhichOne == 3) {
      // Not used in config mode
    } else if (WhichOne == 4) {
      if (AppMode == 0) {
        if (StartupPower > 1) StartupPower --;
      } else {
        if (Kp > 0.1) Kp -= 0.1;
      }
    } else if (WhichOne == 5) {
      if (AppMode == 0) {
        if (FallbackPower > 1) FallbackPower --;
      } else {
        if (Ki > 0.001) Ki -= 0.001;
      }
    } else if (WhichOne == 6) {
      if (AppMode == 0) {
        if (RestPeriod > 1) RestPeriod --;
      } else {
        if (Kd > 0.0) Kd -= 0.1;
      }
    }
  } else {
    if (WhichOne == 0) {
      if (CurrentMode > 1) CurrentMode --;
    } else if (WhichOne == 2) {
      if (UserPower > 1) UserPower --;
    } else if (WhichOne == 3) {
      if (UserTemp1 > 1) UserTemp1 --;
    } else if (WhichOne == 4) {
      if (UserTemp1 > 1) UserTemp1 --;
    } else if (WhichOne == 5) {
      if (UserTemp2 > 1) UserTemp2 --;
    } else if (WhichOne == 6) {
      if (UserTime > 1) UserTime --;
    }
  }
  ScreenUpdate();
}
//-----------------------------------------------------------------------------------------------
void ProcessButton(byte WhichOne) { // Handle increment/decrement button inputs
  int HoldCounter = 0;

  if (WhichOne == 1) {
    // Increment active screen button value by 1
    IncValue(ActiveButton);
    while (digitalRead(INC_BTN) == 0) {
      delay(10);
      HoldCounter ++;
      if (HoldCounter == 150) { // User is intentionally holding the + button
        while (digitalRead(INC_BTN) == 0) {
          IncValue(ActiveButton);
          delay(250);
          HoldCounter ++;
          if ((! ConfigMode) && (ActiveButton == 0) && (HoldCounter > 210)) { // Switch app app mode
            if (AppMode == 0) {
              AppMode = 1;
              PopoverMessage("Now in Micro Boilermaker mode");
            } else {
              AppMode = 0;
              PopoverMessage("Now in Airhead mode");
            }
            SetMemory();
            delay(3000);
            ESP.restart();
          }
        }
      }
    }
  } else {
    // Decrement active screen button value by 1
    DecValue(ActiveButton);
    while (digitalRead(DEC_BTN) == 0) {
      delay(10);
      HoldCounter ++;
      if (HoldCounter == 150) { // User is intentionally holding the - button
        while (digitalRead(DEC_BTN) == 0) {
          DecValue(ActiveButton);
          delay(250);
        }
      }
    }
  }
  UserMode = CurrentMode;
  SetMemory();
}
//-----------------------------------------------------------------------------------------------
void loop() {
  if (UpdateMode) {
    server.handleClient();
    delay(1);
    return;
  }

  bool Resting = false;
  int CurrentPercent = round(0.392156863 * PowerLevel);
  unsigned long CurrentTime = millis();
  if (CurrentTime > 4200000000) {
    // Reboot the system if we're reaching the maximum long integer value of CurrentTime (49 days)
    ESP.restart();
  }
  // Check for touch-screen presses and handle as necessary
  if (GotInterrupt) {
    if (Touch.read()) {
      int Xpos,Ypos;
      TP_Point Point = Touch.getPoint(0);
      Xpos = 320 - Point.y; // Touch.setRotation() doesn't work for some reason
      Ypos = Point.x;       // "                                              "
      ProcessTouch(Xpos,Ypos);
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
    Serial.println("\nRunning Status Updates");
    Serial.print("Current Mode: "); Serial.println(CurrentMode);
    TempUpdate(); // Read the DS18B20 temperature
    // Safety net in case of thermal runaway
    if (TempC > 103) {
      Serial.println("Shutdown due to thermal runaway");
      while (true) {
        RunState(0);
        PopoverMessage("Thermal runaway shutdown");
        delay(2500);
        ScreenUpdate();
        PopoverMessage("Reboot the appliance");
        delay(2500);
        ScreenUpdate();
      }
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
            FallBackTime = millis();
            if (CurrentMode == 3) StartTime = FallBackTime; // Mode 3 runs reset the timer when the minimum operating temperature has been reached
            if (AppMode == 0) {
              if (CurrentPercent > FallbackPower) CurrentPercent = FallbackPower;
              PowerAdjust(CurrentPercent);
            }
          } else {
            if ((AppMode == 0) && (CurrentTime - LastAdjustment >= 60000)) {
              if (CurrentPercent < 100) CurrentPercent ++;
              PowerAdjust(CurrentPercent);
            } else {
              if (myPID.Compute()) PowerAdjust(round(pidOutput));
            }
          }
          if ((CurrentMode == 3) && (long(CurrentTime - StartTime) >= long(UserTime * 3600000))) {
            // Timer expired waiting to reach minimum operating temperature
            RunState(0);
            Serial.print("Failed to reach temperature");
            PopoverMessage("Failed to reach temperature");
            delay(3000);
          }
        } else {
          if ((CurrentTime - FallBackTime >= (RestPeriod * 1000)) || (AppMode == 1)) { // Airhead mode waits for turbulence to calm down after the fall-back
            if (CurrentMode == 2) { // Constant temperature mode
              Serial.print("Target Temp: "); Serial.println(float(UserTemp1),2);
              if (AppMode == 0) {
                if (CurrentTime - LastAdjustment >= 30000) { // Only make power adjustments once every 30 seconds
                  // Temperature is managed to +/- .2 degree C
                  if (TempC >= (UserTemp1 + .2)) { // Over temperature
                    if (CurrentPercent > 0) CurrentPercent --;
                    PowerAdjust(CurrentPercent); // Decrease power 1%
                  } else if (TempC <= (UserTemp1 - .2)) { // Under temperature
                    if (CurrentPercent < 100) CurrentPercent ++;
                    PowerAdjust(CurrentPercent); // Increase power 1%
                  }
                }
              } else {
                if (myPID.Compute()) PowerAdjust(round(pidOutput));
              }
              Serial.print("Power Percent: "); Serial.println(CurrentPercent);
            } else { // Timed distillation run with progressive temperature adjustment
              if (CurrentTime - StartTime < (UserTime * 3600000)) { // 3600000 milliseconds = 1 hour
                if (CurrentTime - Mode3Counter >= 900000) { // Adjust the target temperature every 15 minutes
                  if (Mode3Direction == 1) {
                    Mode3Temp += Mode3Factor; // Increase the target temperature
                  } else {
                    Mode3Temp -= Mode3Factor; // Decrease the target temperature
                  }
                  Mode3Counter = CurrentTime;
                }
                Serial.print("Mode 3 Factor: "); Serial.println(Mode3Factor);
                Serial.print("Target Temp: "); Serial.println(Mode3Temp,2);
                if (AppMode == 0) {
                  if (CurrentTime - LastAdjustment >= 30000) { // Only make power adjustments once every 30 seconds
                    // Temperature is managed to +/- .2 degree C
                    if (TempC >= (Mode3Temp + .2)) { // Over temperature
                      if (CurrentPercent > 0) CurrentPercent --;
                      PowerAdjust(CurrentPercent); // Decrease power 1%
                    } else if (TempC <= (Mode3Temp - .2)) { // Under temperature
                      if (CurrentPercent < 100) CurrentPercent ++;
                      PowerAdjust(CurrentPercent); // Increase power 1%
                    }
                  }
                } else {
                  targetTemp = Mode3Temp;
                  if (myPID.Compute()) PowerAdjust(round(pidOutput));
                }
                Serial.print("Power Percent: "); Serial.println(CurrentPercent);
              } else { // Distillation run time has expired, shut down
                RunState(0);
                Serial.print("Distillation run complete");
                PopoverMessage("Distillation run complete");
                delay(2500);
              }
            }
          } else {
            Resting = true;  
          }
        }
      } else {
        Serial.print("Power Percent: "); Serial.println(CurrentPercent);
      }
    }
    ScreenUpdate();
    if (Resting) PopoverMessage("Rest period now active");
    LoopCounter = CurrentTime;
  }
}
//------------------------------------------------------------------------------------------------
/*
// Create & run a new sketch with the following code to fully erase the flash memory of an ESP32

#include <nvs_flash.h>

void setup() {
  nvs_flash_erase(); // erase the NVS partition and...
  nvs_flash_init();  // initialize the NVS partition.
  while(true);
}

void loop() {

}
*/
//------------------------------------------------------------------------------------------------
