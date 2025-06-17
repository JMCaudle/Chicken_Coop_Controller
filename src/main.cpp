/*
make your own configuration.h file, ie...
#define LATITUDE yourLattitude
#define LONGITUDE yourLongitude
#define DST_OFFSET yourOffset
#define STASSID "your SSID" // set your SSID
#define STAPSK "your Password"      // set your wifi password
#define MY_TZ "your Daylight Savings Code"
*/
#include "configuration.h"

// The SPIFFS (FLASH filing system) is used to hold touch screen
// calibration data
// #include "FS.h"
#include <Preferences.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h> //test removing
#include <WiFiUdp.h> //test removing

#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <WiFi.h>
#include <time.h>
#include <Tasker.h>
#include <sunset.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <functional>
#include <vector>

#include "VeDirectFrameHandler.h"
#include "Tab.h"
#include "DataPoint.h"
#include "DisplayElement.h"
#include "Utility.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
Tasker tasker;
SunSet sun;
Preferences preferences;
VeDirectFrameHandler myVE;

#define TAB_TEXTSIZE 1

#define MY_NTP_SERVER "pool.ntp.org"

#define VERSION "2.3.8"
//------------------------------------------------------------------------------------------

//input pins
const int prPin = 34, closedDoorSensorPin = 39, openDoorSensorPin = 36, btnPin = 22, 
          tempPin = 32;

//output pins
const int lampPin = 15, nightLightPin = 13, bistroLightPin = 12, lockPin = 14, 
          sprayPin = 27, fanPin = 26, motFwd = 25, motRev = 33;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(tempPin);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature tempSensors(&oneWire);

struct Tabs
{
  enum tabs
  {
    General,
    Power,
    Time,
    Temp,
    Util
  };
};

using Utility::ButtonState;

void wifiStart();
void wifiConnect();
void wifiAttemptConnection();
void wifiDisconnectCallback(WiFiEvent_t event, WiFiEventInfo_t info);
void wifiStatus(String status);
void setupOTAUpdates();
// void setupMDNShostname();

void initializeNonVolatileStorage();
void initializePins();
void initializeTabs();
void initializePanels();
DisplayElement* addDisplayElement(DataPoint *dp, int8_t panel, String label, 
                                  String pre = "", String suf = "");
DisplayElement* addUpDownDisplayElement(DataPoint *dp, int8_t panel, String label, 
                                        int min, int max, String pre = "", String suf = "");
DisplayElement* addButtonDisplayElement(DataPoint *dp, int8_t panel,
                                        std::vector<ButtonState *> *buttonStates);
void initializeInteractivity();
void initializeDoorState();
void initializeDayorNightfromTime();
void initializeDayorNightfromLight();

void updateSystemTime();
void resetDefaultSettings();

void drawMainUI();
void drawTabs();
void drawPanelUI();
void drawSysTime();

void updateNightHours();
void updateLightLevels();
// void testFunction();
void updateActiveDoorSensors();
void updateTemperatures();
void readTemperatures();
void readVEData();
void updateVEData();

void ensureValidTouchCalibration();
void touch_calibrate();
void handleTouchInput();
void checkDoorSensors();

void doSunsetJobs();
void doSunriseJobs();
void doFauxSunriseJobs();

void turnNightLightOn();
void turnNightLightOff();
void turnBistroLightsOn();
void turnBistroLightsOff();
void turnMainLightOn();
void turnMainLightOff();

void lockDoor(); //testing
void unlockDoor();  //testing
void openDoor();
void closeDoor();
void manualHaltDoor();
void returnDoor();
void stopDoor();
void windUp();
void windDown();

void resetDoorSensors();

void calcByLight();
void calcByTime();


void startSpray();
void stopSpray();
void enableSpray();
void swapThermPosition();

const uint8_t numTabs = 5;
char tabLabel[numTabs][8] = {"Main", "Power", "Time", "Temp", "Utility"};
TFT_eSPI_Tab tab[numTabs];
uint16_t maxWidth;
uint16_t maxHeight;
uint8_t tabHeight = 50;
uint8_t sysTrayHeight = 40;
uint16_t panelTop = tabHeight * 1.1 + 1;
uint16_t panelBottom = tft.height() - sysTrayHeight;
uint8_t deHeight = 34;
uint8_t openTab = 0;
std::vector<DisplayElement*> panelDEs[numTabs];
uint8_t buttonCounter[numTabs];
uint16_t buttonYpos = panelBottom - 52;
uint16_t statusX, statusY;
uint16_t clockPadding;
uint8_t statusLineSize;
uint8_t statusFontHeight;
char statusBuffer[3][15] = {"", "", ""};
uint8_t connectionAttempts;

time_t now; // this is the seconds since Epoch (1970) - UTC
struct tm tm;      // the structure tm holds time information in a more convenient way *
int currentMin = -1;

unsigned long timeStamp;
unsigned long lastSecondStamp;

// IntData secondsCounter; //testing
StringData activeDoorSensors;
BoolData isDay;
IntData nightLightState;
std::vector<ButtonState *> nightLightBSs;
IntData bistroLightsState;
std::vector<ButtonState *> bistroLightsBSs;


FloatData batCurrent(-5, 10);
FloatData batVoltage;
FloatData loadPower;
FloatData pvPower;
IntData batCapacity;
IntData stateOfCharge(false, 20, 120);
IntData lastSocPrediction;

DoubleData nextSunset(true);
DoubleData nextSunrise(true);
DoubleData nextFauxSunrise(true);
IntData idealNight(true);
IntData artificialNight(true);
IntData lightCutoff;
Utility::RollingAverage avgSunsetLightLevel(10);
DisplayElement* lightCutoffDE;
IntData currentLight;
StringData doorStatus;
StringData doorSensorStatus("Nominal");
IntData mainLightState;
std::vector<ButtonState *> mainLightBSs;
IntData doorState;
std::vector<ButtonState *> doorBSs;
IntData resetDoorSensorsState;
std::vector<ButtonState *> resetDoorSensorsBSs;
DisplayElement *resetDoorSensorsDE;
IntData timeOrLightState;
std::vector<ButtonState *> timeOrLightBSs;

FloatData insideTemp(45, 105);
FloatData outsideTemp(10, 115);
IntData tempCutoff;
IntData sprayDuration;
IntData sprayCooldown;
IntData sprayState;
std::vector<ButtonState *> sprayBSs;
BoolData swapThermometerPositions;
IntData swapThermState;
std::vector<ButtonState *> swapThermStateBSs;

// possibly remove the following test functions...
IntData lockState;
std::vector<ButtonState *> lockStateBSs;
DisplayElement *lockStateDE;
IntData windUpState;
std::vector<ButtonState *> windUpStateBSs;
DisplayElement *windUpDE;
IntData windDownState;
std::vector<ButtonState *> windDownStateBSs;
DisplayElement *windDownDE;
IntData resetDefaultsState;
std::vector<ButtonState *> resetDefaultsBSs;

bool day;
bool dayInit;
bool fauxDay;
unsigned long nightStart = 0;
bool openingDoor;
bool closingDoor;
bool doorOverride;
bool doorLastOpening;
unsigned long doorStartTime;
uint16_t doorOpeningFollowThrough = 2800; // ms
Utility::RollingAverage doorOpeningTimes(10);
IntData avgDoorOpeningTime;
uint16_t doorClosingFollowThrough = 4500; // ms
Utility::RollingAverage doorClosingTimes(10);
IntData avgDoorClosingTime;
unsigned long doorTimeout = 10000; // stop trying to move door if 10 seconds have passed withour success
bool sprayDisabled;

bool batFloat;
double storedAmpSeconds;
// float currentThreshold = 0.09; // |I| < this will be ignored

void setup()
{
  // Use serial0  for Serial Monitor, legacy baud rate should be safe to change
  Serial.begin(9600);

  // VE_direct communication
  Serial2.begin(19200);
  Serial2.flush();

  // Initialise the TFT screen
  tft.init();
  tft.setRotation(2);

  // Calculate SysTray settings
  tft.setFreeFont(CLOCK_FONT);
  tft.setTextSize(1);
  clockPadding = tft.textWidth("88:88 88/88/88");
  tft.setTextFont(0);
  maxHeight = tft.height();
  statusFontHeight = tft.fontHeight();
  statusY = maxHeight - statusFontHeight;
  statusX = maxWidth = tft.width();
  statusLineSize = (tft.width() - clockPadding) / tft.textWidth("W") - 1; // 14

  initializeNonVolatileStorage();

  initializePins();
  initializeTabs();
  initializePanels();
  initializeInteractivity();
  tab[openTab].open(true);
  drawMainUI();

  ensureValidTouchCalibration();

  wifiStart();
/*
    //WiFi.persistent(false);
  //WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Utility::status(".");
    delay(100);
    Utility::status("");
  }
  Utility::status("WiFi connected");
*/
  // might want to remove this to wait for wifi/manual time setting
  // updateSystemTime();

  sun.setPosition(LATITUDE, LONGITUDE, DST_OFFSET);
  if(timeOrLightState.getValue() == 1) // light
  {
    initializeDayorNightfromLight();
  }

  initializeDoorState();

  tempSensors.begin();
  tempSensors.setWaitForConversion(false);

  tasker.setInterval(drawSysTime, 1000);
  // tasker.setInterval(testFunction, 500);
  tasker.setInterval(updateActiveDoorSensors, 1000);
  tasker.setInterval(updateLightLevels, 1000);
  tasker.setInterval(updateTemperatures, 5000);

  //replace these with tasks / methods
  // doorStatus.setValue("Locked Shut");

  lastSecondStamp = millis();

  Utility::status("Version " + (String)VERSION);

  // String test = "9:00";
  // Utility::status("*" + (String)test.toInt() + "*");
}

//------------------------------------------------------------------------------------------

void loop(void)
{
  ArduinoOTA.handle();

  if(digitalRead(btnPin) == LOW)
  {
    touch_calibrate();
  }

  timeStamp = millis();
  checkDoorSensors(); 
  handleTouchInput(); //needs to be called last

  if(dayInit)
  {
    if (timeOrLightState.getValue() == 0)
    {
      if (tm.tm_year > 100)
      {
        double currentTime = Utility::toFractionalMinutes(tm.tm_hour, tm.tm_min, tm.tm_sec);
        // set bool day in setup. Possibly using light levels, otherwise waiting for time to update
        if (day && currentTime >= nextSunset.getValue())
        {
          avgSunsetLightLevel.addData(currentLight.getValue());
          lightCutoff.setValue(avgSunsetLightLevel.getAverage());
          doSunsetJobs();
        }
        else if (!day && currentTime >= nextSunrise.getValue() &&
                 currentTime < nextSunset.getValue())
        {
          doSunriseJobs();
        }
        else if (!(day || fauxDay) && currentTime >= nextFauxSunrise.getValue() &&
                 currentTime < nextSunset.getValue())
        {
          doFauxSunriseJobs();
        }
      }
    }
    else
    {
      // set bool day in setup. Possibly using light levels, otherwise waiting for time to update
      if (day && currentLight.getValue() < lightCutoff.getValue())
      {
        doSunsetJobs();
      }
      else if (!day && currentLight.getValue() > lightCutoff.getValue() &&
               (timeStamp - nightStart > 7 * 60 * 60000 || nightStart == 0)) // atleast 7 hours of night
      {
        doSunriseJobs();
      }
      else if (!(day || fauxDay) && timeStamp - nightStart >= artificialNight.getValue() * 60000)
      {
        doFauxSunriseJobs();
      }
    }
  }  

  if(outsideTemp.getValue() >= tempCutoff.getValue() && !sprayDisabled)
  {
    startSpray();
  }

  // updateVictronData();
  readVEData();

  if (timeStamp - lastSecondStamp > 1000)
  {
    lastSecondStamp = timeStamp;
    updateVEData();
  }

  tasker.loop();
}

//------------------------------------------------------------------------------------------

void initializeNonVolatileStorage()
{
  // Initialize Non-Volatile Data Storage
  preferences.begin("ChickenCoop", false);
  // preferences.clear(); // uncomment to reset to defaults, probably put this behind a button
  batCapacity.makeDataPersist("batCapacity", 20);
  if (preferences.isKey("charge"))
  {
    // Needs different treatment since this is not a DataPoint
    storedAmpSeconds = preferences.getDouble("charge", 0);
  }

  idealNight.makeDataPersist("idealNight", 540);
  artificialNight.makeDataPersist("artificialNight", 660);
  lightCutoff.makeDataPersist("lightCutoff", 150);
  avgSunsetLightLevel.initializeData(lightCutoff.getValue());
  doorSensorStatus.makeDataPersist("doorSensorStatus", "Nominal");
  timeOrLightState.makeDataPersist("TorLstate", false);

  tempCutoff.makeDataPersist("tempCutoff", 100);
  sprayDuration.makeDataPersist("sprayDuration", 2);
  sprayCooldown.makeDataPersist("sprayCooldown", 20);
  swapThermometerPositions.makeDataPersist("swapThermometer", false);

  // Uncomment when in situ
  avgDoorOpeningTime.makeDataPersist("avgDoorOpen", 45000);
  doorOpeningTimes.initializeData(avgDoorOpeningTime.getValue());
  avgDoorClosingTime.makeDataPersist("avgDoorClose", 45000);
  doorClosingTimes.initializeData(avgDoorClosingTime.getValue());
}

void initializePins()
{
  // ADC prPin doesn't require initialization
  // and tempPin is initialized by the OneWire constructor
  pinMode(btnPin, INPUT_PULLUP);
  pinMode(openDoorSensorPin, INPUT);
  pinMode(closedDoorSensorPin, INPUT);

  pinMode(lampPin, OUTPUT);
  pinMode(nightLightPin, OUTPUT);
  pinMode(bistroLightPin, OUTPUT);
  pinMode(lockPin, OUTPUT);
  digitalWrite(lockPin, HIGH);
  pinMode(sprayPin, OUTPUT);
  digitalWrite(sprayPin, HIGH);
  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, HIGH);
  pinMode(motFwd, OUTPUT);
  pinMode(motRev, OUTPUT);
}

void initializeTabs()
{
  for (uint8_t i = 0; i < numTabs; i++)
  {
    uint8_t tabWidth = tft.width() / numTabs;
    tab[i].initTabUL(&tft, i * tabWidth, tabHeight * 0.1,
                     tabWidth, tabHeight,
                     TFT_WHITE, TFT_LIGHTGREY, TFT_DARKGREY, TFT_WHITE,
                     tabLabel[i], (GFXfont *)ACTIVE_FONT, 
                     (GFXfont *)INACTIVE_FONT, TAB_TEXTSIZE);
  }
}

void initializePanels()
{
  // *** General Panel Display Elements ***
  // addDisplayElement(&secondsCounter, Tabs::General, "Seconds:");
  addDisplayElement(&isDay, Tabs::General, "Day?:");
  addDisplayElement(&stateOfCharge, Tabs::General, "Battery Charge", "", "%");
  addDisplayElement(&nextSunset, Tabs::General, "Nightfall");
  addDisplayElement(&nextSunrise, Tabs::General, "Sunrise");
  addDisplayElement(&insideTemp, Tabs::General, "Inside Temp.", "", " F");
  addDisplayElement(&outsideTemp, Tabs::General, "Outside Temp.", "", " F");
  nightLightBSs.push_back(new ButtonState(turnNightLightOn, "Night Light Off", "Turn On")); // 0
  nightLightBSs.push_back(new ButtonState(turnNightLightOff, "Night Light On", "Turn Off")); // 1
  addButtonDisplayElement(&nightLightState, Tabs::General, &nightLightBSs);
  bistroLightsBSs.push_back(new ButtonState(turnBistroLightsOn, "Bistro Lights Off", "Turn On")); // 0
  bistroLightsBSs.push_back(new ButtonState(turnBistroLightsOff, "Bistro Lights On", "Turn Off")); // 1
  addButtonDisplayElement(&bistroLightsState, Tabs::General, &bistroLightsBSs);  
  doorBSs.push_back(new ButtonState(openDoor, "Door Closed", "Open Door"));  // 0
  doorBSs.push_back(new ButtonState(manualHaltDoor, "Door Opening", "Stop Door")); // 1
  doorBSs.push_back(new ButtonState(closeDoor, "Door Open", "Close Door"));        // 2
  doorBSs.push_back(new ButtonState(manualHaltDoor, "Door Closing", "Stop Door")); // 3
  doorBSs.push_back(new ButtonState(returnDoor, "Door Stopped", "Return Door"));   // 4
  addButtonDisplayElement(&doorState, Tabs::General, &doorBSs);

  // *** Power Panel Display Elements ***
  addDisplayElement(&batCurrent, Tabs::Power, "Battery Current", "", "A");
  addDisplayElement(&batVoltage, Tabs::Power, "System Voltage", "", "V");
  addDisplayElement(&loadPower, Tabs::Power, "Power Consumption", "", "W");
  addDisplayElement(&pvPower, Tabs::Power, "Power Generation", "", "W");
  addUpDownDisplayElement(&batCapacity, Tabs::Power, "Battery Capacity", 0, 1000, "", "Ah");
  addDisplayElement(&stateOfCharge, Tabs::Power, "Battery Charge", "", "%");
  addDisplayElement(&lastSocPrediction, Tabs::Power, "Predicted SoC @ full", "", "%");

  // *** Time Panel Display Elements ***
  addDisplayElement(&nextSunset, Tabs::Time, "Nightfall");
  addDisplayElement(&nextSunrise, Tabs::Time, "Sunrise");
  addUpDownDisplayElement(&idealNight, Tabs::Time, "Ideal Night", 6, 12);//420, 720);
  addUpDownDisplayElement(&artificialNight, Tabs::Time, "Achieved Night", 6, 12); // 420, 720);
  lightCutoffDE = addUpDownDisplayElement(&lightCutoff, Tabs::Time, "Light Cutoff", 0, 4095);
  addDisplayElement(&currentLight, Tabs::Time, "Light Level");
  // addDisplayElement(&doorStatus, Tabs::Time, "Door");
  addDisplayElement(&doorSensorStatus, Tabs::Time, "Door Sensors");
  mainLightBSs.push_back(new ButtonState(turnMainLightOn, "Main Light Off", "Turn On")); // 0
  mainLightBSs.push_back(new ButtonState(turnMainLightOff, "Main Light On", "Turn Off")); // 1
  addButtonDisplayElement(&mainLightState, Tabs::Time, &mainLightBSs);
  // doorBSs.push_back(new ButtonState(openDoor, "Door Closed", "Open Door"));  // 0
  // doorBSs.push_back(new ButtonState(manualHaltDoor, "Door Opening", "Stop Door")); // 1
  // doorBSs.push_back(new ButtonState(closeDoor, "Door Open", "Close Door")); // 2
  // doorBSs.push_back(new ButtonState(manualHaltDoor, "Door Closing", "Stop Door")); // 3
  // doorBSs.push_back(new ButtonState(returnDoor, "Door Stopped", "Return Door")); // 4
  addButtonDisplayElement(&doorState, Tabs::Time, &doorBSs);
  resetDoorSensorsBSs.push_back(new ButtonState(resetDoorSensors, "Reset Door Sensors"));
  resetDoorSensorsDE = addButtonDisplayElement(&resetDoorSensorsState, Tabs::Time, &resetDoorSensorsBSs);
  timeOrLightBSs.push_back(new ButtonState(calcByLight, "Night From Time", "Use Light")); // 0
  timeOrLightBSs.push_back(new ButtonState(calcByTime, "Night From Light", "Use Time")); // 1
  addButtonDisplayElement(&timeOrLightState, Tabs::Time, &timeOrLightBSs);

  // ***Temp Panel Display Elements***
  addDisplayElement(&insideTemp, Tabs::Temp, "Inside Temp.", "", " F");
  addDisplayElement(&outsideTemp, Tabs::Temp, "Outside Temp.", "", " F");
  addUpDownDisplayElement(&tempCutoff, Tabs::Temp, "Spray at Temp.", 80, 120, "", " F");
  addUpDownDisplayElement(&sprayDuration, Tabs::Temp, "Spray Duration", 0, 30, "", "m");
  addUpDownDisplayElement(&sprayCooldown, Tabs::Temp, "Spray Cooldown", 0, 1440, "", "m");
  sprayBSs.push_back(new ButtonState(startSpray, "Spray is Off", "Start Spray"));
  sprayBSs.push_back(new ButtonState(stopSpray, "Spray is On", "Stop Spray"));
  addButtonDisplayElement(&sprayState, Tabs::Temp, &sprayBSs);
  swapThermStateBSs.push_back(new ButtonState(swapThermPosition, "Swap Thermo Labels"));
  addButtonDisplayElement(&swapThermState, Tabs::Temp, &swapThermStateBSs);

  // ***Utility Panel Display Elements***
  addDisplayElement(&activeDoorSensors, Tabs::Util, "Active Door Sensors");
  lockStateBSs.push_back(new ButtonState(unlockDoor, "Door Locked", "Unlock"));  // 0
  lockStateBSs.push_back(new ButtonState(lockDoor, "Door Unlocked", "Lock")); // 1
  lockStateDE = addButtonDisplayElement(&lockState, Tabs::Util, &lockStateBSs);
  windUpStateBSs.push_back(new ButtonState(windUp, "Wind Up"));  // 0
  windUpStateBSs.push_back(new ButtonState(stopDoor, "Stop")); // 1
  windUpDE = addButtonDisplayElement(&windUpState, Tabs::Util, &windUpStateBSs);
  windDownStateBSs.push_back(new ButtonState(windDown, "Wind Down"));  // 0
  windDownStateBSs.push_back(new ButtonState(stopDoor, "Stop")); // 1
  windDownDE = addButtonDisplayElement(&windDownState, Tabs::Util, &windDownStateBSs);  
  resetDefaultsBSs.push_back(new ButtonState(resetDefaultSettings, "Reset To Defaults"));
  addButtonDisplayElement(&resetDefaultsState, Tabs::Util, &resetDefaultsBSs);
}

DisplayElement *addDisplayElement(DataPoint *dp, int8_t panel, String label, String pre, String suf)
{
  DisplayElement *de = new DisplayElement(&tft, dp, label, 0, 
    panelTop + 20 + (deHeight * panelDEs[panel].size()), maxWidth, deHeight, pre, suf);
  panelDEs[panel].push_back(de);
  return de;
}

DisplayElement* addUpDownDisplayElement(DataPoint *dp, int8_t panel, String label, int min, int max, String pre, String suf)
{
  DisplayElement *de = new UpDownElement(&tft, dp, label, 0,
    panelTop + 20 + (deHeight * panelDEs[panel].size()), maxWidth, deHeight, min, max, pre, suf);
  panelDEs[panel].push_back(de);
  return de;
}

DisplayElement *addButtonDisplayElement(DataPoint *dp, int8_t panel, std::vector<ButtonState *> *buttonStates)
{
  buttonCounter[panel] += 1;
  DisplayElement *de = new ButtonElement(&tft, dp, 42 + 78 * (buttonCounter[panel] - 1), 
                                          buttonYpos, 72, 64, buttonStates);
  panelDEs[panel].push_back(de);
  return de;
}

void initializeInteractivity()
{
  // lightCutoff should only be interactive if timeOrLight = 1 (light)
  lightCutoffDE->setInteractivity(timeOrLightState.getValue() != 0);

  resetDoorSensorsDE->setInteractivity(doorSensorStatus.getValue() != "Nominal");

  if (lockState.getValue() == 0)
  {
    windUpDE->setInteractivity(false);
    windDownDE->setInteractivity(false);
  }
}

void initializeDayorNightfromTime()
{
  double currentTime = Utility::toFractionalMinutes(tm.tm_hour, tm.tm_min, tm.tm_sec);
  if (currentTime >= nextSunrise.getValue() && currentTime < nextSunset.getValue())
  {
    day = true;
    isDay.setValue(true);
  }
  else
  {
    day = false;
    isDay.setValue(false);
  }
  dayInit = true; //hack, find better solution
}

void initializeDayorNightfromLight()
{
  if (currentLight.getValue() >= lightCutoff.getValue())
  {
    day = true;
    isDay.setValue(true);
  }
  else
  {
    day = false;
    isDay.setValue(false);
  }
  dayInit = true; // hack, find better solution
}

void initializeDoorState()
{
  if (digitalRead(openDoorSensorPin) == LOW && digitalRead(closedDoorSensorPin) == HIGH)
  {
    doorState.setValue(2); // open
  }
  else if (digitalRead(closedDoorSensorPin) == LOW && digitalRead(openDoorSensorPin) == HIGH)
  {
    doorState.setValue(0); //closed
  }
  else if (digitalRead(closedDoorSensorPin) == HIGH && digitalRead(openDoorSensorPin) == HIGH)
  {
    doorOverride = true;
    doorTimeout = avgDoorClosingTime.getValue();
    closeDoor();
    // possibly display somewhere that we performed a blind closure
  }
  else
  {
    // Error State, should never get here
    Utility::status("Both door sensors active.");
  }
}

/*--------------------------------------------------------------*/
// GUI functions

void drawMainUI()
{
  tft.fillScreen(TFT_BLACK);
  drawTabs();
  drawPanelUI();
}

void drawTabs()
{
  for (uint8_t i = 0; i < numTabs; i++)
  {
    tab[i].drawTab();
  }
}

void drawPanelUI()
{
  tft.fillRect(0, panelTop, tft.width(), panelBottom - panelTop, TFT_LIGHTGREY);
  tft.drawFastVLine(0, panelTop, panelBottom - panelTop, TFT_WHITE);
  tft.drawFastVLine(tft.width() - 1, panelTop - 4, panelBottom - panelTop + 4, TFT_WHITE);
  tft.drawFastHLine(0, panelBottom, tft.width(), TFT_WHITE);
  Utility::setPanelTextSettings();

  for (int i = 0; i < panelDEs[openTab].size(); i++)
  {
      panelDEs[openTab][i]->drawLabel();
      panelDEs[openTab][i]->linkUpDataPoint();
      panelDEs[openTab][i]->getDataPoint()->processValue();
  }
}

void drawSysTime()
{
  time(&now);             // read the current time
  localtime_r(&now, &tm); // update the structure tm with the current time
  if(currentMin != tm.tm_min)
  { // only update display when min changes
    tft.setTextPadding(clockPadding);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(CLOCK_FONT);
    tft.setTextDatum(CL_DATUM);
    char hourStr[3];
    sprintf(hourStr, "%02d", tm.tm_hour);
    char minStr[3];
    sprintf(minStr, "%02d", tm.tm_min);
    String timeString = (String)hourStr + ":" + (String)minStr + " " +
                        (String)(tm.tm_mon + 1) + "/" + (String)tm.tm_mday +
                        "/" + (String)(tm.tm_year % 100);
    tft.drawString(timeString, 5, (panelBottom + tft.height()) / 2);
    currentMin = tm.tm_min;
  }
}
//------------------------------------------------------------------------------------------

void handleTouchInput()
{
  uint16_t t_x = 0, t_y = 0; // To store the touch coordinates
  // Pressed will be set true is there is a valid touch on the screen
  bool pressed = tft.getTouch(&t_x, &t_y);
  if (pressed)
  {
    // Is touch in tab UI?
    if (t_y <= tabHeight * 1.1)
    {
      for (uint8_t t = 0; t < numTabs; t++)
      {
        // Is touch on specific tab?
        if (tab[t].contains(t_x, t_y))
        {
          // don't keep redrawing the same tab/panel
          if (openTab != t)
          {
            // deactivates DisplayElements on current panel
            for (uint8_t i = 0; i < panelDEs[openTab].size(); i++)
            {
              panelDEs[openTab][i]->unlinkDataPoint();
            }
            // redraw tabs to account for new openTab
            for (uint8_t i = 0; i < numTabs; i++)
            {
              tab[i].open(i == t);
              tab[i].drawTab();
            }
            openTab = t;
            drawPanelUI();
          }
          break;
        }
        // Touch was in tab UI, but not on a tab, do nothing
      }
    }
    else
    {
      // Touch is on Panel UI - check each element on the open panel
      for (int8_t i = 0; i < panelDEs[openTab].size(); i++)
      {
        panelDEs[openTab][i]->handleButtonTouchInput(t_x, t_y);
      }
    }
    // delay(10);  // possibly needed for UI debouncing
  }
  else
  {
    for (int8_t i = 0; i < panelDEs[openTab].size(); i++)
    {
      panelDEs[openTab][i]->noTouch();
    }
  }
}

void checkDoorSensors()
{
  if (openingDoor)
  {
    // Broke these into nested ifs to check if this was a good run for the rolling average
    if (timeStamp - doorStartTime >= doorTimeout)
    {
      stopDoor();
      openingDoor = false;
      doorState.setValue(2);
      if (doorSensorStatus.getValue() == "!Bottom!" || doorSensorStatus.getValue() == "!Both!")
      {
        doorSensorStatus.setValue("!Both!");
      }
      else
      {
        doorSensorStatus.setValue("!Top!");
      }
      resetDoorSensorsDE->setInteractivity(true);
      doorOverride = false;
    }
    else if (digitalRead(openDoorSensorPin) == LOW)
    {
      tasker.setTimeout(stopDoor, doorOpeningFollowThrough);
      openingDoor = false;
      doorState.setValue(2);
      if (!doorOverride)
      {
        doorOpeningTimes.addData((double) (millis() - doorStartTime + doorOpeningFollowThrough));
        avgDoorOpeningTime.setValue((int)doorOpeningTimes.getAverage());
      }
      doorOverride = false;
    }
  }  
  else if (closingDoor)
  {
    if (timeStamp - doorStartTime >= doorTimeout)
    {
      stopDoor();
      closingDoor = false;
      doorState.setValue(0);
      if (doorSensorStatus.getValue() == "!Top!" || doorSensorStatus.getValue() == "!Both!")
      {
        doorSensorStatus.setValue("!Both!");
      }
      else
      {
        doorSensorStatus.setValue("!Bottom!");
      }
      resetDoorSensorsDE->setInteractivity(true);
      doorOverride = false;
    }
    else if (digitalRead(closedDoorSensorPin) == LOW)
    {
      tasker.setTimeout(stopDoor, doorClosingFollowThrough);
      closingDoor = false;
      doorState.setValue(0);
      if (!doorOverride)
      {
        doorClosingTimes.addData((double)(millis() - doorStartTime + doorClosingFollowThrough));
        avgDoorClosingTime.setValue((int)doorClosingTimes.getAverage());
      }
      doorOverride = false;
    }
  }
}

void ensureValidTouchCalibration()
{
  preferences.end();
  preferences.begin("TouchCal", false);
  uint16_t calData[5];

  if (preferences.isKey("calData") && preferences.getBytesLength("calData") == 10)
  {
    preferences.getBytes("calData", calData, 10);
    tft.setTouch(calData);
  }
  else
  {
    touch_calibrate();
  }
  preferences.end();
  preferences.begin("ChickenCoop", false);
}

void touch_calibrate()
{  
  preferences.end();
  preferences.begin("TouchCal", false);
  uint16_t calData[5];
  if (preferences.isKey("calData"))
  {
    preferences.remove("calData");
  }

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 0);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.println("Touch corners as indicated");

  tft.setTextFont(1);
  tft.println();

  tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("Calibration complete!");

  // store data
  preferences.putBytes("calData", calData, 10);

  preferences.end();
  preferences.begin("ChickenCoop", false);

  drawMainUI();
}

void updateSystemTime()
{
  configTime(0, 0, MY_NTP_SERVER); // 0, 0 because we will use TZ in the next line
  setenv("TZ", MY_TZ, 1);          // Set environment variable with your time zone
  tzset();
}

void readVEData()
{
  while (Serial2.available())
  {
    myVE.rxData(Serial2.read());
  }
  yield();
}

void updateVEData()
{
  for (int i = 0; i < myVE.veEnd; i++)
  {
    if (strcmp(myVE.veName[i], "I") == 0)
    {
      batCurrent.setValue(atof(myVE.veValue[i]) / 1000); // mA to A
    }
    else if (strcmp(myVE.veName[i], "V") == 0)
    {
      batVoltage.setValue(atof(myVE.veValue[i]) / 1000); // mA to A
    }
    else if (strcmp(myVE.veName[i], "IL") == 0)
    {
      loadPower.setValue((atof(myVE.veValue[i]) / 1000) * batVoltage.getValue()); // mA to A
    }
    else if (strcmp(myVE.veName[i], "PPV") == 0)
    {
      pvPower.setValue(atof(myVE.veValue[i])); 
    }
    else if (strcmp(myVE.veName[i], "CS") == 0)
    {
      uint8_t state = atoi(myVE.veValue[i]);
      if (state == 5) // Charger is in "Float" state, so battery is fully charged
      {
        storedAmpSeconds = batCapacity.getValue() * 3600; // Ah to As
        if(!batFloat)
        {
          lastSocPrediction.setValue(stateOfCharge.getValue());
          batFloat = true;
        }
      }
      else
      {
        // Since this is being called once per second, our present current is approximately
        // equal to the change in stored Amp*Seconds
        // probably want to account for temperature, and maybe Peukert's law
        // For now assuming 95% charge efficiency
        batFloat = false;
        float current = batCurrent.getValue();
        if (abs(current) > 0.09) // Ignore current below threshold
        {
          if (current > 0) // charging
          {
            storedAmpSeconds += current * 0.95;
          }
          else
          {
            storedAmpSeconds += current;
          }
        }
      }
      stateOfCharge.setValue((storedAmpSeconds / (batCapacity.getValue() * 3600)) * 100);
    }
  }
}

/*
void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin())
  {
    Serial.println("formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE))
  {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f)
      {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL)
  {
    // calibration data valid
    tft.setTouch(calData);
  }
  else
  {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL)
    {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f)
    {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}
*/
//------------------------------------------------------------------------------------------

void doSunsetJobs()
{
  nightStart = timeStamp;
  closeDoor();
  day = false;
  isDay.setValue(false);
}

void doSunriseJobs()
{
  openDoor();
  day = true;
  isDay.setValue(true);
  tasker.setTimeout(updateNightHours, 300000); //update in 5 minutes
  turnMainLightOff();
  fauxDay = false;
}

void doFauxSunriseJobs()
{
  fauxDay = true;
  turnMainLightOn();
  artificialNight.setValue(max(idealNight.getValue(), artificialNight.getValue() - 3));
}

void updateNightHours()
{
  if (tm.tm_year < 100)
  {
    tasker.setTimeout(updateNightHours, 5000);
    return;
  }

  sun.setCurrentDate(1900 + tm.tm_year, tm.tm_mon, tm.tm_mday);
  double set = sun.calcNauticalSunset();
  double rise = sun.calcNauticalSunrise();
  if (tm.tm_isdst > 0)
  {
    set += 60;
    rise += 60;
  }
  nextSunset.setValue(set);
  nextSunrise.setValue(rise);
  nextFauxSunrise.setValue(((int)set + artificialNight.getValue()) % 1440);
}

void updateLightLevels()
{
  currentLight.setValue(analogRead(prPin));
}

// void testFunction()
// {
//   secondsCounter.setValue(tm.tm_sec);
// }

void resetDefaultSettings()
{
  preferences.clear();
  ESP.restart();
}

void updateActiveDoorSensors()
{
  if (digitalRead(openDoorSensorPin) == LOW)
  {
    if(digitalRead(closedDoorSensorPin) == LOW)
    {
      activeDoorSensors.setValue("!Both!");
    }
    else
    {
      activeDoorSensors.setValue("Top");
    }
  }
  else
  {
    if(digitalRead(closedDoorSensorPin) == HIGH)
    {
      activeDoorSensors.setValue("!None!");
    }
    else
    {
      activeDoorSensors.setValue("Bottom");
    }
  }
}

void turnNightLightOn()
{
  digitalWrite(nightLightPin, HIGH);
  nightLightState.setValue(1);
}

void turnNightLightOff()
{
  digitalWrite(nightLightPin, LOW);
  nightLightState.setValue(0);
}

void turnBistroLightsOn()
{
  digitalWrite(bistroLightPin, HIGH);
  bistroLightsState.setValue(1);
}

void turnBistroLightsOff()
{
  digitalWrite(bistroLightPin, LOW);
  bistroLightsState.setValue(0);
}

void turnMainLightOn()
{
  digitalWrite(lampPin, HIGH);
  mainLightState.setValue(1);
}

void turnMainLightOff()
{
  digitalWrite(lampPin, LOW);
  mainLightState.setValue(0);
}

//probably remove these, testing purposes
void unlockDoor()
{
  digitalWrite(lockPin, LOW);
  lockState.setValue(1);
  windUpDE->setInteractivity(true);
  windDownDE->setInteractivity(true);
}

void lockDoor()
{
  digitalWrite(lockPin, HIGH);
  lockState.setValue(0);
  windUpDE->setInteractivity(false);
  windDownDE->setInteractivity(false);
}

void openDoor()
{
  if(digitalRead(openDoorSensorPin) == LOW && !doorOverride)
  {
    doorState.setValue(2);
    return;
  }
  // digitalWrite(lockPin, LOW);      // unlock
  unlockDoor();
  tasker.setTimeout(windUp, 200); // begin raising door after 0.2 seconds (for lock)
  // doorStartTime = millis();  // now setting in windUp
  doorState.setValue(1);
  // openingDoor = true;   //now in windup
  doorLastOpening = true;
  if (!doorOverride)
  {
    doorTimeout = avgDoorOpeningTime.getValue();
  }
}

void closeDoor()
{
  if (digitalRead(closedDoorSensorPin) == LOW && !doorOverride)
  {
    doorState.setValue(0);
    return;
  }
  // digitalWrite(lockPin, LOW);        // unlock
  unlockDoor();
  tasker.setTimeout(windDown, 200); // begin closing door after 0.2 seconds
  // doorStartTime = millis();  // now setting in windDown
  doorState.setValue(3);
  // closingDoor = true;  // now setting in windDown
  doorLastOpening = false;
  if (!doorOverride)
  {
    doorTimeout = avgDoorClosingTime.getValue();
  }
}

// manual override door stop function
void manualHaltDoor()
{
  stopDoor();
  if (!doorOverride)
  {
    // an approximation, might want to modify with a constant for irl conditions
    doorTimeout = millis() - doorStartTime; 
    //actually this is probably unnecessary
  }
  else
  {
    if (doorLastOpening)
    {
      // how long it takes to close 
      // minus how long it was going to take to raise 
      // plus how long we were moving
      doorTimeout = avgDoorClosingTime.getValue() 
                    - doorTimeout 
                    + (millis() - doorStartTime);
    }
    else
    {
      doorTimeout = avgDoorOpeningTime.getValue() 
                    - doorTimeout 
                    + (millis() - doorStartTime);
    }
  }
  doorOverride = true;
  doorState.setValue(4);
}

void returnDoor()
{
  if (doorLastOpening)
  {
    closeDoor();
  }
  else
  {
    openDoor();
  }
}

void stopDoor()
{
  openingDoor = closingDoor = false;
  digitalWrite(motFwd, LOW);
  digitalWrite(motRev, LOW);
  // digitalWrite(lockPin, HIGH); // locked
  lockDoor();
  lockStateDE->setInteractivity(true);
  windUpState.setValue(0); // probs remove, testing
  windDownState.setValue(0); // probs remove, testing
}

void windUp()
{
  lockStateDE->setInteractivity(false);
  digitalWrite(motFwd, HIGH);
  digitalWrite(motRev, LOW);
  doorStartTime = millis();
  openingDoor = true;
  windUpState.setValue(1); // probs remove, testing
}

void windDown()
{
  lockStateDE->setInteractivity(false);
  digitalWrite(motFwd, LOW);
  digitalWrite(motRev, HIGH);
  doorStartTime = millis();
  closingDoor = true;
  windDownState.setValue(1); //probs remove, testing
}

void resetDoorSensors()
{
  doorSensorStatus.setValue("Nominal");
  resetDoorSensorsDE->setInteractivity(false);
}

void calcByLight()
{
  timeOrLightState.setValue(1);
  lightCutoffDE->setInteractivity(true);
}

void calcByTime()
{
  timeOrLightState.setValue(0);
  lightCutoffDE->setInteractivity(false);
}

void updateTemperatures()
{
    tempSensors.requestTemperatures();
    tasker.setTimeout(readTemperatures, 1000);
}

void readTemperatures()
{
  if(swapThermometerPositions.getValue())
  {
    outsideTemp.setValue(tempSensors.getTempFByIndex(1));
    insideTemp.setValue(tempSensors.getTempFByIndex(0));
  }
  else
  {
  outsideTemp.setValue(tempSensors.getTempFByIndex(0));
  insideTemp.setValue(tempSensors.getTempFByIndex(1));
  }
}

void startSpray()
{
  digitalWrite(sprayPin, LOW);
  sprayState.setValue(1);
  tasker.setTimeout(stopSpray, 60000 * sprayDuration.getValue());
  sprayDisabled = true;
}

void stopSpray()
{
  digitalWrite(sprayPin, HIGH);
  sprayState.setValue(0);
  tasker.setTimeout(enableSpray, 60000 * sprayCooldown.getValue());
}

void enableSpray()
{
  sprayDisabled = false;
}


void swapThermPosition()
{
  swapThermometerPositions.setValue(!swapThermometerPositions.getValue());
}

///////////////////////////////////////////////////////

void wifiStart()
{
  WiFi.disconnect(true);
  delay(1000);
  WiFi.onEvent(wifiDisconnectCallback, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  wifiConnect();
}

void wifiConnect()
{
  // WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  tasker.setInterval(wifiAttemptConnection, 100);
}

void wifiAttemptConnection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
  
    if(connectionAttempts > 210)
    {
      WiFi.disconnect();
      WiFi.begin(STASSID, STAPSK);
      connectionAttempts = 0;
    }
    
    else
    {
      if (connectionAttempts > 200)
      {
        wifiStatus("Retrying Wifi");
      }
      else
      {
        wifiStatus(connectionAttempts % 2 ? "Connecting  " : "Connecting .");
      }
      connectionAttempts++;
    }
  }
  else
  {
    wifiStatus("Wifi Connected");
    connectionAttempts = 0;
    
    updateSystemTime();
    // tasker.setTimeout(updateNightHours, 5000);
    // tasker.setTimeout(initializeDayorNight, 6000); //possibly refactor as a single function
    // tasker.setTimeout(initializeDoorState, 7000); //that can be called with/out WiFi
    updateNightHours();
    if(timeOrLightState.getValue() == 0)
    {
      initializeDayorNightfromTime();
    }
    setupOTAUpdates();
    tasker.cancel(wifiAttemptConnection);
    }
}

void wifiDisconnectCallback(WiFiEvent_t event, WiFiEventInfo_t info)
{
  wifiConnect();
}

void wifiStatus(String status)
{
  uint8_t tempSize = tft.textsize;
  uint8_t tempDatum = tft.getTextDatum();
  uint16_t tempPadding = tft.getTextPadding();

  tft.setTextPadding(maxWidth - clockPadding);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(0);
  tft.setTextDatum(BR_DATUM);
  tft.setTextSize(1);
  tft.drawString(status, maxWidth, maxHeight);

  tft.setTextDatum(tempDatum);
  tft.setTextSize(tempSize);
  tft.setTextPadding(tempPadding);
}

void setupOTAUpdates()
{
  //cribbed from ArduinoOTA/examples/basicOTA.ino
  ArduinoOTA.setHostname("ChickenCoop");

  ArduinoOTA
      .onStart([]()
               {
      // This might be the place to call a function for saving data with preferences.h
      // For Data that changes too frequently to constantly save updates
      preferences.putDouble("charge", storedAmpSeconds);

      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.setPassword("Xb476X3aU");
  ArduinoOTA.begin();
}

