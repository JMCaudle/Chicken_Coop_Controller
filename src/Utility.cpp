#include "Utility.h"

extern uint16_t statusX, statusY;
extern uint16_t clockPadding;
extern uint8_t statusLineSize;
extern uint8_t statusFontHeight;
extern char statusBuffer[3][15];
extern TFT_eSPI tft;

// using namespace Utility;

void Utility::setPanelTextSettings(bool alarm)
{
  tft.setFreeFont(DISPLAY_ELEMENT_FONT);
  if(alarm)
  {
    tft.setTextColor(TFT_RED, TFT_LIGHTGREY);
  }
  else
  {
    tft.setTextColor(TFT_WHITE, TFT_LIGHTGREY);
  }

}

String Utility::minutesToTimeString(int mins)
{
  return fractionalMinutesToTimeString((double)mins);
}

String Utility::fractionalMinutesToTimeString(double fM)
{
  uint8_t hours = std::floor(fM / 60);
  uint8_t mins = std::round(fM - (hours * 60));
  char hourStr[3];
  sprintf(hourStr, "%02d", hours);
  char minStr[3];
  sprintf(minStr, "%02d", mins);
  return (String)hourStr + ":" + (String)minStr;
}

double Utility::toFractionalMinutes(int hour, int min, int sec) 
{
  return /*(double)*/(hour * 60) + min + ((double)sec / 60);
}

void addLineToStatusBuffer(String line)
{
  Serial.println("Line from addLineToStatusBuffer: " + line);
  // statusBuffer[2] = statusBuffer[1];
  // statusBuffer[1] = statusBuffer[0];
  // statusBuffer[0] = line;
  strcpy(statusBuffer[2], statusBuffer[1]);
  strcpy(statusBuffer[1], statusBuffer[0]);
  strcpy(statusBuffer[0], line.c_str());
}

void Utility::status(const char *msg)
{
  Utility::status((String)msg);
}

void Utility::status(String msg)
{
  uint8_t tempSize = tft.textsize;
  uint8_t tempDatum = tft.getTextDatum();
  uint16_t tempPadding = tft.getTextPadding();

  tft.setTextPadding(tft.width() - clockPadding);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(0);
  tft.setTextDatum(BR_DATUM);
  tft.setTextSize(1);
  String message = msg;
  Serial.println("Whole message from status: " + message);

  // while (message.length() > statusLineSize)
  // {
  //   int idx = message.indexOf(" ", message.length() - statusLineSize);
  //   if (idx == -1)
  //   {
  //     // found no spaces for linebreak
  //     idx = message.length() - statusLineSize;
  //     addLineToStatusBuffer(message.substring(idx));
  //     // tft.drawString(message.substring(idx), statusX, statusY - (statusFontHeight * rows));
  //   }
  //   else
  //   {
  //     addLineToStatusBuffer(message.substring(idx + 1));
  //     // tft.drawString(message.substring(idx + 1), statusX, statusY - (statusFontHeight * rows));
  //   }
  //   message.remove(idx);
  //   // rows++;
  // }

  message.trim();
  while (message.length() > statusLineSize)
  {
    int idx = message.indexOf(" ");
    if (idx == -1 || idx >= statusLineSize)
    {
      // found no spaces for linebreak
      idx = statusLineSize;
      addLineToStatusBuffer(message.substring(0, idx));
      message.remove(0, idx);
      // tft.drawString(message.substring(idx), statusX, statusY - (statusFontHeight * rows));
    }
    else
    {
      addLineToStatusBuffer(message.substring(0, idx));
      message.remove(0, idx + 1);
      // tft.drawString(message.substring(idx + 1), statusX, statusY - (statusFontHeight * rows));
    }
    // addLineToStatusBuffer(message.substring(0, idx));
    // message.remove(0, idx);
    // rows++;
    message.trim();
  }
  addLineToStatusBuffer(message);

  for (uint8_t row = 0; row < 3; row++)
  {
    tft.drawString(statusBuffer[row], statusX, statusY - (statusFontHeight * row));
  }
    

  tft.setTextDatum(tempDatum);
  tft.setTextSize(tempSize);
  tft.setTextPadding(tempPadding);
}

