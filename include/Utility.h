#ifndef UTIL
#define UTIL
#include <TFT_eSPI.h>

namespace Utility
{
#define INACTIVE_FONT &FreeSans9pt7b // inactive tab label
#define ACTIVE_FONT &FreeSansBold9pt7b // active tab label
#define DISPLAY_ELEMENT_FONT &FreeSans12pt7b
#define BUTTON_FONT &FreeSans9pt7b
#define CLOCK_FONT &FreeSans18pt7b

#define TFT_MIDGREY 0xA514

  struct ButtonState
  {
    std::function<void(void)> btnFunction;
    String btnText;
    String btnLabel;

    ButtonState(std::function<void(void)> bf, String bt, String bl = "")
        : btnFunction(bf), btnText(bt), btnLabel(bl) {};
  };

  void setPanelTextSettings();

  String fractionalMinutesToTimeString(double fM);
  double toFractionalMinutes(int hour, int min, int sec);

  void status(const char *msg);
  void status(String msg);
}

#endif
