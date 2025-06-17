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
  _interactive = true;
}

void DisplayElement::linkUpDataPoint()
{
  _dp->setCallback([&](String s,  bool a){ this->drawValue(s, a); });
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
  Utility::setPanelTextSettings();
  _dataPadding = (_w - 10) - _gfx->drawString(_label, _x + 5, _y + (_h / 2));
}

void DisplayElement::drawValue(String val, bool alarm)
{
  // Utility::status("In the BASE drawValue method.");
  _gfx->setTextDatum(CR_DATUM);
  Utility::setPanelTextSettings(alarm);
  _gfx->setTextPadding(_dataPadding);
  _gfx->drawString(_prefix + val + _suffix, _x + _w - 5, _y + (_h / 2));
}

void DisplayElement::setInteractivity(bool interactive)
{
  _interactive = interactive;
  _dp->processValue();
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
  if(!_interactive)
  {
    return;
  }
  
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
                              int w, int h, int min, int max, String pre, String suf)
: DisplayElement(gfx, dp, label, x, y, w, h, pre, suf)
{
  _lastValWidth = 0;
  _min = min;
  _max = max;
  IntData *idp = (IntData *)dp;
  buttons[0] = &minusBtn;
  buttons[0]->setMyFunction([idp]()
                            { idp->setValue(idp->getValue() - 1); });

  buttons[1] = &plusBtn;
  buttons[1]->setMyFunction([idp]()
                            { idp->setValue(idp->getValue() + 1); });
  }

void UpDownElement::drawValue(String val, bool alarm)
{
  _gfx->setTextDatum(CR_DATUM);
  Utility::setPanelTextSettings();
  _gfx->setTextPadding(_dataPadding - (_bw + 2 * _bs));
  String catStr = _prefix + val + _suffix;
  _gfx->fillRect(_gfx->width() - _dataPadding, _y, _dataPadding - 1, _h, TFT_LIGHTGREY);
  _gfx->drawString(catStr, _x + _w - (_bw + 2 * _bs), _y + (_h / 2));
  uint16_t valWidth = _gfx->textWidth(catStr);
  // uint16_t valWidth = catStr.length() * _gfx->textWidth("8");
  // uint16_t valWidth = catStr.length() * _gfx->textWidth("W");

  // if (val.compareTo())

  bool interactive = _interactive && val.toInt() < _max;
  uint16_t fill = interactive ? TFT_WHITE : TFT_MIDGREY;
  // uint16_t fill = _interactive ? TFT_WHITE : TFT_MIDGREY;

  plusBtn.initButton(_gfx, _x + _w - (_bw + _bs), _y + (_h / 2), _bw, _bh,
                     TFT_BLACK, fill, TFT_LIGHTGREY, true, interactive);
  plusBtn.drawButton();

  interactive = _interactive && val.toInt() > _min;
  fill = interactive ? TFT_WHITE : TFT_MIDGREY;
  minusBtn.initButton(_gfx, _x + _w - (_bw + 3 * _bs) - valWidth, _y + (_h / 2), _bw, _bh,
                      TFT_BLACK, fill, TFT_LIGHTGREY, false, interactive);
  minusBtn.drawButton();
  Utility::setPanelTextSettings();
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

void ButtonElement::drawValue(String val, bool alarm)
{
  int stIdx = val.toInt(); // investigate a switch to std::string

  // also investigate passing references instead of pointers
  btn.setMyFunction((*_btnStates)[stIdx]->btnFunction);
  btn.drawButton(_interactive, (*_btnStates)[stIdx]->btnText, (*_btnStates)[stIdx]->btnLabel);
  //should not need to set inverted to true, confused
}
