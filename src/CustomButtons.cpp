#include "CustomButtons.h"

CustomButton::CustomButton(void)
{
  _gfx = nullptr;
  _xd = 0;
  _yd = 0;
  _textdatum = MC_DATUM;
  _label = "";
  currstate = false;
  laststate = false;
}

// Classic initButton() function: pass center & size
void CustomButton::initButton(
    TFT_eSPI *gfx, int16_t x, int16_t y, uint16_t w, uint16_t h,
    uint16_t outline, uint16_t fill, uint16_t textcolor,
    String label, uint8_t textsize)
{
  // Tweak arguments and pass to the newer initButtonUL() function...
  initButtonUL(gfx, x - (w / 2), y - (h / 2), w, h, outline, fill,
               textcolor, label, textsize);
}

// Newer function instead accepts upper-left corner & size
void CustomButton::initButtonUL(
    TFT_eSPI *gfx, int16_t x1, int16_t y1, uint16_t w, uint16_t h,
    uint16_t outline, uint16_t fill, uint16_t textcolor,
    String label, uint8_t textsize)
{
  _x = x1;
  _y = y1;
  _w = w;
  _h = h;
  _outlinecolor = outline;
  _fillcolor = fill;
  _textcolor = textcolor;
  _textsize = textsize;
  _gfx = gfx;
  _label = label;
  // strncpy(_label, label, 9);
}

// Adjust text datum and x, y deltas
void CustomButton::setLabelDatum(int16_t x_delta, int16_t y_delta, uint8_t datum)
{
  _xd = x_delta;
  _yd = y_delta;
  _textdatum = datum;
}

void CustomButton::drawButton(bool inverted, String long_name)
{
  uint16_t fill, outline, text;

  if (!inverted)
  {
    fill = _fillcolor;
    outline = _outlinecolor;
    text = _textcolor;
  }
  else
  {
    fill = _textcolor;
    outline = _outlinecolor;
    text = _fillcolor;
  }

  uint8_t r = min(_w, _h) / 4; // Corner radius
  _gfx->fillRoundRect(_x, _y, _w, _h, r, fill);
  _gfx->drawRoundRect(_x, _y, _w, _h, r, outline);

  _gfx->setTextColor(text, fill);
  _gfx->setTextSize(_textsize);
  _gfx->setFreeFont(BUTTON_FONT);

  uint8_t tempdatum = _gfx->getTextDatum();
  _gfx->setTextDatum(_textdatum);
  uint16_t tempPadding = _gfx->getTextPadding();
  _gfx->setTextPadding(0);

  String label = (long_name == "" ? _label : long_name);

  uint8_t rows = 0;
  uint16_t centerX = _x + (_w / 2) + _xd;
  uint16_t centerY = _y + (_h / 2) - 4 + _yd;
  uint8_t labelLineSize = _w / _gfx->textWidth("W") - 1;
  uint8_t lineHeight = _gfx->fontHeight();
  std::vector<String> subStrings;

  while (label.length() > labelLineSize)
  {
    int idx = label.indexOf(" ", label.length() - labelLineSize);
    if (idx == -1)
    {
      // found no spaces for linebreak
      idx = label.length() - labelLineSize;
      subStrings.push_back(label.substring(idx));
      // _gfx->drawString(label.substring(idx), statusX, statusY - (_gfx->fontHeight() * rows));
    }
    else
    {
      subStrings.push_back(label.substring(idx + 1));
      // _gfx->drawString(label.substring(idx + 1), statusX, statusY - (_gfx->fontHeight() * rows));
    }
    label.remove(idx);
    rows++;
  }
  subStrings.push_back(label); // might want to check for empty string

  for (uint8_t i = 0; i < subStrings.size(); i++)
  {
    _gfx->drawString(subStrings[i], centerX, centerY + lineHeight * rows / -2 + lineHeight * i);
  }
    // _gfx->drawString(label, statusX, statusY - (_gfx->fontHeight() * rows));

    // _gfx->drawString(label, _x + (_w / 2) + _xd, _y + (_h / 2) - 4 + _yd);

  _gfx->setTextDatum(tempdatum);
  _gfx->setTextPadding(tempPadding);
}

bool CustomButton::contains(int16_t x, int16_t y)
{
  return ((x >= _x) && (x < (_x + _w)) &&
          (y >= _y) && (y < (_y + _h)));
}

void CustomButton::press(bool p)
{
  laststate = currstate;
  currstate = p;
}

bool CustomButton::isPressed() { return currstate; }
bool CustomButton::justPressed() { return (currstate && !laststate); }
bool CustomButton::justReleased() { return (!currstate && laststate); }

void IncrementButton::initButton(TFT_eSPI *gfx, int16_t x, int16_t y,
                                 uint16_t w, uint16_t h, uint16_t outline, uint16_t fill,
                                 uint16_t textcolor, bool positive) 
{
  _positive = positive;
  _x = x;
  _y = y;
  _w = w;
  _h = h;
  _outlinecolor = outline;
  _fillcolor = fill;
  _textcolor = textcolor;
  _gfx = gfx;

  _v1.x = x;
  _v1.y = y + (h / 2);
  _v2.x = x;
  _v2.y = y - (h / 2);
  _v3.x = x + (_positive ? w : - w);
  _v3.y = y;
}

void IncrementButton::drawButton(bool inverted, String long_name)
{
  uint16_t fill, outline, text;

  if (!inverted)
  {
    fill = _fillcolor;
    outline = _outlinecolor;
    text = _textcolor;
  }
  else
  {
    fill = _textcolor;
    outline = _outlinecolor;
    text = _fillcolor;
  }

  _gfx->fillTriangle(_v1.x, _v1.y, _v2.x, _v2.y, _v3.x, _v3.y, fill);
  _gfx->drawTriangle(_v1.x, _v1.y, _v2.x, _v2.y, _v3.x, _v3.y, outline);

  int16_t symbolX = _x + _xd + (_positive ? _w * 5 / 16 : _w * -5 / 16);
  int8_t symbolSize = (_h * 2 / 5) | 1; //bitwise OR to garuantee oddness
  // draws -
  for (int8_t i = 0; i < 3; i++)
  {
    _gfx->drawFastHLine(symbolX - (symbolSize / 2), _y + (i - 1), symbolSize, text);
  }
  //draws the | of +
  if(_positive)
  {
    for (int8_t i = 0; i < 3; i++)
    {
      _gfx->drawFastVLine(symbolX + (i - 1), _y - (symbolSize / 2), symbolSize, text);
    }
  }
    

  // _gfx->setTextColor(text, fill);

  // uint8_t tempSize = _gfx->textsize;
  // _gfx->setTextSize(_textsize);
  // uint8_t tempdatum = _gfx->getTextDatum();
  // _gfx->setTextDatum(_textdatum);
  // uint16_t tempPadding = _gfx->getTextPadding();
  // _gfx->setTextPadding(0);

  // center is (3/8)w from ref point
  // int16_t offset = _positive ? _w * 5 / 16 : _w * -5 / 16;

  // _gfx->drawString(_label, _x + offset + _xd, _y + _yd);

  // _gfx->setTextDatum(tempdatum);
  // _gfx->setTextPadding(tempPadding);
  // _gfx->setTextSize(tempSize);
}

bool IncrementButton::contains(int16_t x, int16_t y)
{
  // https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
  int as_x = x - _v1.x;
  int as_y = y - _v1.y;

  bool s_ab = (_v2.x - _v1.x) * as_y - (_v2.y - _v1.y) * as_x > 0;

  if ((_v3.x - _v1.x) * as_y - (_v3.y - _v1.y) * as_x > 0 == s_ab)
    return false;
  if ((_v3.x - _v2.x) * (y - _v2.y) - (_v3.y - _v2.y) * (x - _v2.x) > 0 != s_ab)
    return false;
  return true;
}
