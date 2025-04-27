#ifndef DISP_ELEM
#define DISP_ELEM
#include <TFT_eSPI.h>
#include "Utility.h"
#include "DataPoint.h"
#include "CustomButtons.h"

using Utility::ButtonState; //could possibly conflict with a future namespace

//template <typename T>
class DisplayElement
{
public:
  DisplayElement(TFT_eSPI* gfx, DataPoint* dp, String label, int x, int y, int w, int h, 
                  String pre = "", String suf = "");

  void linkUpDataPoint();
  void unlinkDataPoint();
  DataPoint* getDataPoint();

  virtual void drawLabel();
  virtual void drawValue(String val);

  void setInteractivity(bool interactive);
  void noTouch();
  void handleButtonTouchInput(uint16_t t_x, uint16_t t_y);
  CustomButton *buttons[2] = {nullptr, nullptr};

protected:
  TFT_eSPI *_gfx;
  DataPoint *_dp;
  String _label, _prefix, _suffix;
  uint16_t _x, _y, _w, _h;
  uint8_t _dataPadding;
  bool _interactive;
};

class UpDownElement : public DisplayElement
{
public:
  UpDownElement(TFT_eSPI* gfx, DataPoint* dp, String label, int x, int y, int w, int h,
                 String pre = "", String suf = "");
  void drawValue(String val);
  IncrementButton plusBtn, minusBtn;
protected:
  int16_t _lastValWidth; // not currently using
  int8_t _bw = 26; // Button Width
  int8_t _bh = 24; // Button Height
  int8_t _bs = 5;  // Button Spacing
};

class ButtonElement : public DisplayElement
{
public:
  // ButtonElement(TFT_eSPI *gfx, DataPoint *dp, int x, int y, int w, int h,
  //               String btnText, String btnAlt = "", String label = "");
  ButtonElement(TFT_eSPI *gfx, DataPoint *dp, int x, int y, int w, int h,
                std::vector<ButtonState*>* btnStates);
  CustomButton btn;
  void drawLabel();
  void drawValue(String val = "");
  std::function<void(void)> falseFunction;
  std::function<void(void)> trueFunction;

private:
  int _resetTime;
  String _btnText;
  String _btnAlt;
  std::vector<ButtonState*>* _btnStates;
};

#endif
