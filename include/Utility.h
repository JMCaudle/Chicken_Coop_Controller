#ifndef UTIL
#define UTIL
#include <TFT_eSPI.h>
#include <vector>

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

  class RollingAverage
  {
  public:
    RollingAverage(int windowSize) : 
      windowSize_(windowSize), 
      data_(windowSize, 0.0), 
      index_(0) {}

    void initializeData(double initialValue)
    {
      for(uint8_t i = 0; i < windowSize_; i++)
      {
        data_[i] = initialValue;
      }
    }

    void addData(double value)
    {
      data_[index_] = value;                // Add the new value
      index_ = (index_ + 1) % windowSize_; // Move to the next index in the circular buffer
    }

    double getAverage() const
    {
      double sum = 0;
      for (uint8_t i = 0; i < windowSize_; i++)
      {
        sum += data_[i];
      }
      return sum / windowSize_;
    }

  private:
    int windowSize_;
    std::vector<double> data_;
    int index_;
  };

  void setPanelTextSettings(bool alarm = false);

  String minutesToTimeString(int min);
  String fractionalMinutesToTimeString(double fM);
  double toFractionalMinutes(int hour, int min, int sec);

  // void addLineToStatusBuffer(String line);
  void status(const char *msg);
  void status(String msg);

}

#endif
