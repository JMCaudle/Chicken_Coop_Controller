#include "Utility.h"

extern uint16_t statusX, statusY;
extern uint16_t clockPadding;
extern uint8_t statusLineSize;
extern TFT_eSPI tft;


void Utility::setPanelTextSettings()
{
  tft.setFreeFont(DISPLAY_ELEMENT_FONT);
  tft.setTextColor(TFT_WHITE, TFT_LIGHTGREY);
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

void Utility::status(const char *msg)
{
  Utility::status((String)msg);
}

void Utility::status(String msg)
{
  uint8_t tempSize = tft.textsize;
  uint8_t tempDatum = tft.getTextDatum();
  uint16_t tempPadding = tft.getTextPadding();
  // GFXfont* tempFont = tft.gfxFont;
  tft.setTextPadding(tft.width() - clockPadding);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(0);
  tft.setTextDatum(BR_DATUM);
  tft.setTextSize(1);
  String message = msg;
  uint8_t rows = 0;
  while (message.length() > statusLineSize)
  {
    int idx = message.indexOf(" ", message.length() - statusLineSize);
    if (idx == -1)
    {
      // found no spaces for linebreak
      idx = message.length() - statusLineSize;
      tft.drawString(message.substring(idx), statusX, statusY - (tft.fontHeight() * rows));
    }
    else
    {
      tft.drawString(message.substring(idx + 1), statusX, statusY - (tft.fontHeight() * rows));
    }
    message.remove(idx);
    rows++;
  }
  tft.drawString(message, statusX, statusY - (tft.fontHeight() * rows));

  // tft.setTextFont(tempFont);
  tft.setTextDatum(tempDatum);
  tft.setTextSize(tempSize);
  tft.setTextPadding(tempPadding);
}
