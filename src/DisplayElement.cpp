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
  _dp->setCallback([&](String s){ this->drawValue(s); });
}

void DisplayElement::unlinkDataPoint()
{
  _dp->clearCallback();
}

DataPoint* DisplayElement::getDataPoint()
{
  return _dp;
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

void DisplayElement::noTouch()
{
  // each element has up to 2 buttons
  for (int8_t j = 0; j < 2; j++)
  {
    // Does this element have a button?
    if (buttons[j] != nullptr)
    {    
      buttons[j]->press(false);    
    }
    else
    {
      break; // If no first button, don't check for second button
    }
  }
}

void DisplayElement::handleButtonTouchInput(uint16_t t_x, uint16_t t_y)
{
  // each element has up to 2 buttons
  for (int8_t j = 0; j < 2; j++)
  {
    // Does this element have a button?
    if (buttons[j] != nullptr)
    {
      // Did that button contain the touch?
      if (buttons[j]->contains(t_x, t_y))
      {
        buttons[j]->press(true);
        if (buttons[j]->justPressed())
        {
          buttons[j]->runMyFunction();
          // should this button repeat whilst held down?
          if (buttons[j]->repeatDelay > 0)
          {
            buttons[j]->timePressed = millis();
            buttons[j]->repeatCounter = 0;
          }
        }
        if (buttons[j]->repeatDelay > 0)
        {
          uint16_t elapsedTime = millis() - buttons[j]->timePressed;
          uint16_t currentDelay;
          if(buttons[j]->repeatCounter < 5)
          {
            currentDelay = buttons[j]->repeatDelay;
          }
          else
          {
            currentDelay = buttons[j]->repeatDelay / 5;
          }
          if (elapsedTime > currentDelay)
          {
            buttons[j]->runMyFunction();
            buttons[j]->timePressed = millis();
            buttons[j]->repeatCounter += 1;
          }
        }
      }
      else
      {
        buttons[j]->press(false);
      }
    }
    else
    {
      break; // If no first button, don't check for second button
    }
  }
}

// Can only be used with DataPoint::IntData
UpDownElement::UpDownElement(TFT_eSPI *gfx, DataPoint *dp, String label, int x, int y,
                              int w, int h, String pre, String suf)
: DisplayElement(gfx, dp, label, x, y, w, h, pre, suf)
{
  _lastValWidth = 0;
  IntData *idp = (IntData *)dp;
  buttons[0] = &minusBtn;
  buttons[0]->setMyFunction([idp]()
                            { idp->setValue(idp->getValue() - 1); });

  buttons[1] = &plusBtn;
  buttons[1]->setMyFunction([idp]()
                            { idp->setValue(idp->getValue() + 1); });
  }

void UpDownElement::drawValue(String val)
{
  // Utility::status("In the UDE drawValue method.");

  _gfx->setTextDatum(CR_DATUM);
  Utility::setPanelTextSettings();
  _gfx->setTextPadding(_dataPadding - (_bw + 2 * _bs));
  String catStr = _prefix + val + _suffix;
  _gfx->fillRect(_gfx->width() - _dataPadding, _y, _dataPadding - 1, _h, TFT_LIGHTGREY);
  _gfx->drawString(catStr, _x + _w - (_bw + 2 * _bs), _y + (_h / 2));
  // int16_t valWidth = _gfx->textWidth(catStr);
  // Utility::status((String)valWidth);
  int16_t valWidth = catStr.length() * _gfx->textWidth("8");

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
  btn.initButton(gfx, x, y, w, h, TFT_BLACK, TFT_DARKGREY, TFT_WHITE, _btnText, 1);
  buttons[0] = &btn;
}

ButtonElement::ButtonElement(TFT_eSPI *gfx, DataPoint *dp, int x, int y, int w, int h,
                             std::vector<ButtonState *> *btnStates)
    : DisplayElement(gfx, dp, "", x, y, w, h)
{
  _btnStates = btnStates;
  btn.initButton(gfx, x, y, w, h, TFT_BLACK, TFT_DARKGREY, TFT_WHITE, "", 1);
  buttons[0] = &btn;
}

void ButtonElement::drawLabel()
{

}

void ButtonElement::drawValue(String val)
{
  int stIdx = val.toInt(); // investigate a switch to std::string

  // also investigate passing references instead of pointers
  btn.setMyFunction((*_btnStates)[stIdx]->btnFunction);
  btn.drawButton(true, (*_btnStates)[stIdx]->btnText, (*_btnStates)[stIdx]->btnLabel);
  //should not need to set inverted to true, confused
  // Utility::status((String)_y);
}
