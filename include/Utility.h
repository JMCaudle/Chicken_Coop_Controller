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
      sum_(0.0), 
      index_(0) {}

    void addData(double value)
    {
      sum_ -= data_[index_];                // Subtract the oldest value
      data_[index_] = value;                // Add the new value
      sum_ += value;                        // Update the sum
      index_ = (index_ + 1) % windowSize_; // Move to the next index in the circular buffer
    }

    double getAverage() const
    {
      return sum_ / windowSize_;
    }

  private:
    int windowSize_;
    std::vector<double> data_;
    double sum_;
    int index_;
  };
  
  void setPanelTextSettings();

  String minutesToTimeString(int min);
  String fractionalMinutesToTimeString(double fM);
  double toFractionalMinutes(int hour, int min, int sec);

  // void addLineToStatusBuffer(String line);
  void status(const char *msg);
  void status(String msg);

}

#endif
