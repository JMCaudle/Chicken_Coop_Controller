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
#include "FS.h"

#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <WiFi.h>
#include <time.h>
#include <EEPROM.h>
#include <Tasker.h>
#include <sunset.h>
#include <functional>
#include <vector>
// #include <variant>

#include "Tab.h"
#include "DataPoint.h"
#include "DisplayElement.h"
#include "Utility.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
Tasker tasker;
SunSet sun;

// This is the file name used to store the calibration data
// You can change this to create new calibration files.
// The SPIFFS file name must start with "/".
#define CALIBRATION_FILE "/TouchCalData2"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

#define TAB_TEXTSIZE 1

#define PR_PIN 36

#define MY_NTP_SERVER "pool.ntp.org"
//------------------------------------------------------------------------------------------


struct Tabs
{
  enum tabs
  {
    General,
    Power,
    Time,
    Temp
  };
};

// enum GeneralElements
// {

// };

void initializePanels();
void addDisplayElement(DataPoint *dp, String label, int8_t panel);
void addUpDownDisplayElement(DataPoint *dp, String label, int8_t panel);
void handleTouchInput();

void drawTabs();
void drawPanelUI();
void drawSysTime();

void updateNightHours();
void updateLightLevels();
void testFunction();

void touch_calibrate();

const uint8_t numTabs = 4;
char tabLabel[numTabs][8] = {"General", "Power", "Time", "Temp"};
TFT_eSPI_Tab tab[numTabs];
uint16_t width;
uint8_t tabHeight = 50;
uint8_t sysTrayHeight = 40;
uint16_t panelTop = tabHeight * 1.1 + 1;
uint16_t panelBottom = tft.height() - sysTrayHeight;
uint8_t deHeight = 34;
uint8_t openTab = 0;
std::vector<DisplayElement*> panelDEs[numTabs];
uint16_t statusX, statusY;
uint16_t clockPadding;
uint8_t statusLineSize;

time_t now; // this is the seconds since Epoch (1970) - UTC
struct tm tm;      // the structure tm holds time information in a more convenient way *
int currentMin = -1;
bool day;

unsigned long timeStamp;
unsigned long lastSecondStamp;

DoubleData nextSunset(true);
DoubleData nextSunrise(true);
DoubleData fauxSunrise(true);
IntData idealNight;
IntData nextNight;
IntData lightCutoff;
IntData currentLight;
StringData doorStatus;
StringData doorSensorStatus;

IntData secondsCounter;




void setup()
{
  // Use serial port
  Serial.begin(9600);

  // Initialise the TFT screen
  tft.init();

  // Set the rotation before we calibrate
  tft.setRotation(2);

  // Calibrate the touch screen and retrieve the scaling factors
  touch_calibrate();

  // Calculate SysTray settings
  statusX = tft.width();
  statusY = tft.height();
  tft.setFreeFont(CLOCK_FONT);
  tft.setTextSize(1);
  clockPadding = tft.textWidth("88:88 88/88/88");
  tft.setTextFont(0);
  statusLineSize = (tft.width() - clockPadding) / tft.textWidth("W") - 1;

  // Clear the screen
  tft.fillScreen(TFT_BLACK);

  openTab = Tabs::Time; // remove this later
  width = tft.width();
  initializePanels();
  tab[openTab].open(true);
  drawTabs();
  drawPanelUI();

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

  configTime(0, 0, MY_NTP_SERVER); // 0, 0 because we will use TZ in the next line
  setenv("TZ", MY_TZ, 1);          // Set environment variable with your time zone
  tzset();

  sun.setPosition(LATITUDE, LONGITUDE, DST_OFFSET);

  tasker.setInterval(drawSysTime, 1000);
  tasker.setInterval(testFunction, 500);
  tasker.setInterval(updateNightHours, 5000);  //run this at morning, no tasker
  tasker.setInterval(updateLightLevels, 1000);
  //replace these with tasks
  doorStatus.setValue("Locked Open");
  doorSensorStatus.setValue("Nominal");

  // lastSecondStamp = millis();
}

//------------------------------------------------------------------------------------------

void loop(void)
{
  handleTouchInput();

  // possibly wrap this block in an if statement, with the option to run from light levels
  double currentTime = Utility::toFractionalMinutes(tm.tm_hour, tm.tm_min, tm.tm_sec);
  if(tm.tm_year > 100)
  {
    // set bool day in setup. Possibly using light levels, otherwise waiting for time to update
    if(day && currentTime >= nextSunset.getValue())
    {
      // Do Sunset Jobs
    }
    else if (!day && currentTime >= nextSunrise.getValue())
    {
      // Do Sunrise Jobs
    }
    else if (!day && currentTime >= fauxSunrise.getValue())
    {
      // do Supplemental Lighting jobs
    }
  }

  // timeStamp = millis();
  // if (timeStamp - lastSecondStamp > 1000)
  // {
  //   lastSecondStamp = timeStamp;
  //   testFunction();
  // }

  tasker.loop();
}

//------------------------------------------------------------------------------------------

void initializePanels()
{
  // *** General Panel Display Elements ***
  addDisplayElement(&secondsCounter, "Seconds:", Tabs::General);

  // *** Power Panel Display Elements ***
  // TBD, almost all basic DisplayElements
  // With the possibility of a few buttons on the bottom

  // *** Time Panel Display Elements ***
  addDisplayElement(&nextSunset, "Nightfall", Tabs::Time);
  addDisplayElement(&nextSunrise, "Sunrise", Tabs::Time);
  addUpDownDisplayElement(&idealNight, "Ideal Night", Tabs::Time);
  addUpDownDisplayElement(&nextNight, "Next Night", Tabs::Time);
  addUpDownDisplayElement(&lightCutoff, "Light Cutoff", Tabs::Time);
  addDisplayElement(&currentLight, "Light Level", Tabs::Time);
  addDisplayElement(&doorStatus, "Door", Tabs::Time);
  addDisplayElement(&doorSensorStatus, "Door Sensors", Tabs::Time);
  // Reset Door Sensor Status Non-Toggle Button
  // Main Light               Toggle Button / timer
  // Night Light              Toggle Button / timer
  // Door                     Toggle Button / no timer
  // Time/Light               Toggle Button / no timer

  // ***Temp Panel Display Elements***
  // Indoor Temp              Display Element
  // Outdoor Temp             Display Element
  // Spray Temp Cutoff        UpDown
  // Spray Duration           UpDown
  // Spray Cooldown           UpDown
  // Force Spray              Toggle Button / timer

}

void addDisplayElement(DataPoint* dp, String label, int8_t panel)
{
  DisplayElement *de = new DisplayElement(&tft, dp, label, 0, 
    panelTop + 20 + (deHeight * panelDEs[panel].size()), width, deHeight);
  panelDEs[panel].push_back(de);
}

void addUpDownDisplayElement(DataPoint *dp, String label, int8_t panel)
{
  DisplayElement *de = new UpDownElement(&tft, dp, label, 0,
    panelTop + 20 + (deHeight * panelDEs[panel].size()), width, deHeight);
  panelDEs[panel].push_back(de);
}

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
      // Touch is on Panel UI
    }
    // delay(10);  // possibly needed for UI debouncing
  }
}

void drawTabs()
{
  int tabColor;
  for (uint8_t i = 0; i < numTabs; i++){
    uint8_t tabWidth = tft.width() / numTabs;
    tab[i].initTabUL(&tft, i * tabWidth, tabHeight * 0.1,
                     tabWidth, tabHeight,
                     TFT_WHITE, TFT_LIGHTGREY, TFT_DARKGREY, TFT_WHITE,
                     tabLabel[i], (GFXfont *)ACTIVE_FONT, 
                     (GFXfont *)INACTIVE_FONT, TAB_TEXTSIZE);
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
  }
}

// TODO - break this up into updateSysTime and drawSysTime
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

//------------------------------------------------------------------------------------------

// might need to check if (tm_year != 69) before running
void updateNightHours()
{
  sun.setCurrentDate(1900 + tm.tm_year, tm.tm_mon, tm.tm_mday);
  nextSunset.setValue(sun.calcNauticalSunset());
  nextSunrise.setValue(sun.calcNauticalSunrise());
}

void updateLightLevels()
{
  currentLight.setValue(analogRead(PR_PIN));
}

void testFunction()
{
  secondsCounter.setValue(tm.tm_sec);
}
