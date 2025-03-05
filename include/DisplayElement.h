#ifndef DISP_ELEM
#define DISP_ELEM
#include <TFT_eSPI.h>
// #include <functional>
#include "Utility.h"
#include "DataPoint.h"
#include "CustomButtons.h"

//template <typename T>
class DisplayElement
{
public:
  DisplayElement(TFT_eSPI* gfx, DataPoint* dp, String label, int x, int y, int w, int h, 
                  String pre = "", String suf = "");

  virtual void linkUpDataPoint();
  void unlinkDataPoint();

  virtual void drawLabel();
  virtual void drawValue(String val);

protected:
  TFT_eSPI *_gfx;
  DataPoint *_dp;
  String _label, _prefix, _suffix;
  uint16_t _x, _y, _w, _h;
  uint8_t _dataPadding;
};


class UpDownElement : public DisplayElement
{
public:
  UpDownElement(TFT_eSPI* gfx, DataPoint* dp, String label, int x, int y, int w, int h,
                 String pre = "", String suf = "");
  void linkUpDataPoint();
  void drawValue(String val);
  IncrementButton plusBtn, minusBtn;
protected:
  int16_t _lastValWidth; // not currently using
  int8_t _bw = 26; // Button Width
  int8_t _bh = 24; // Button Height
  int8_t _bs = 5;  // Button Spacing
  // const GFXfont *_ibLabelFont = &FreeSansBold12pt7b; // Increment Button Label Font
};

class ButtonElement : public DisplayElement
{
public:
  ButtonElement(TFT_eSPI* gfx, DataPoint* dp, int x, int y, int w, int h,
                String btnText, String btnAlt = "", String label = "");
  CustomButton btn;
  std::function<void(void)> activationFunction;
  std::function<void(void)> deActivationFunction;

private:
  int _resetTime;
  String _btnText;
  String _btnAlt;
};

#endif
