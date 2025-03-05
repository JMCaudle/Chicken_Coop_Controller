#include <DisplayElement.h>


DisplayElement::DisplayElement(TFT_eSPI* gfx, DataPoint* dp, String label, int x, int y, int w,
                                int h, String pre, String suf)
{
  _gfx = gfx;
  _dp = dp;
  _label = label;
  _prefix = pre;
  _suffix = suf;
  _x = x;
  _y = y;
  _w = w;
  _h = h;
}

void DisplayElement::linkUpDataPoint()
{
  _dp->setAndUseCallback([&](String s){ this->drawValue(s); });
}

void DisplayElement::unlinkDataPoint()
{
  _dp->clearCallback();
}

void DisplayElement::drawLabel()
{
  _gfx->setTextDatum(CL_DATUM);
  _dataPadding = (_w - 10) - _gfx->drawString(_label, _x + 5, _y + (_h / 2));
}

void DisplayElement::drawValue(String val)
{
  // Utility::status("In the BASE drawValue method.");
  _gfx->setTextDatum(CR_DATUM);
  Utility::setPanelTextSettings();
  _gfx->setTextPadding(_dataPadding);
  _gfx->drawString(_prefix + val + _suffix, _x + _w - 5, _y + (_h / 2));
}

// Pass all parameters to Base constructor
UpDownElement::UpDownElement(TFT_eSPI *gfx, DataPoint *dp, String label, int x, int y,
                             int w, int h, String pre, String suf)
    : DisplayElement(gfx, dp, label, x, y, w, h, pre, suf) { _lastValWidth = 0; }

void UpDownElement::linkUpDataPoint() 
{
  Utility::status("In the UDE lUDP method");
  _dp->setAndUseCallback([&](String s){ this->UpDownElement::drawValue(s); });
}

void UpDownElement::drawValue(String val)
{
  Utility::status("In the UDE drawValue method.");

  _gfx->setTextDatum(CR_DATUM);
  Utility::setPanelTextSettings();
  _gfx->setTextPadding(_dataPadding - (_bw + 2 * _bs));
  String catStr = _prefix + val + _suffix;
  _gfx->drawString(catStr, _x + _w - (_bw + 2 * _bs), _y + (_h / 2));
  int16_t valWidth = _gfx->textWidth(catStr);
  // Utility::status((String)valWidth);

  plusBtn.initButton(_gfx, _x + _w - (_bw + _bs), _y + (_h / 2), _bw, _bh,
                     TFT_BLACK, TFT_WHITE, TFT_LIGHTGREY, true);
  plusBtn.drawButton();
  minusBtn.initButton(_gfx, _x + _w - (_bw + 3 * _bs) - valWidth, _y + (_h / 2), _bw, _bh,
                      TFT_BLACK, TFT_WHITE, TFT_LIGHTGREY, false);
  minusBtn.drawButton();
  Utility::setPanelTextSettings();
}

ButtonElement::ButtonElement(TFT_eSPI *gfx, DataPoint *dp, int x, int y, int w, int h,
                             String btnText, String btnAlt, String label)
    : DisplayElement(gfx, dp, label, x, y, w, h)
{
  _btnText = btnText;
  _btnAlt = btnAlt;
}
