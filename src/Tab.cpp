#include <Tab.h>

/***************************************************************************************
** Code for the GFX Tab UI element
** Grabbed from Adafruit_GFX library and enhanced to handle any label font
***************************************************************************************/
TFT_eSPI_Tab::TFT_eSPI_Tab(void)
{
  _gfx = nullptr;
  _xd = 0;
  _yd = 0;
  _textdatum = MC_DATUM;
  _label[9] = '\0';
  currstate = false;
  laststate = false;
}

// Classic initTab() function: pass center & size
void TFT_eSPI_Tab::initTab(TFT_eSPI *gfx, int16_t x, int16_t y,
                           uint16_t w, uint16_t h, uint16_t outline, uint16_t activeFill,
                           uint16_t inactiveFill, uint16_t textcolor, char *label,
                           GFXfont *activeFont, GFXfont *inactiveFont,
                           uint8_t textsize)
{
  // Tweak arguments and pass to the newer initTabUL() function...
  initTabUL(gfx, x - (w / 2), y - (h / 2), w, h, outline, activeFill, inactiveFill,
               textcolor, label, activeFont, inactiveFont, textsize);
}

// Newer function instead accepts upper-left corner & size
void TFT_eSPI_Tab::initTabUL(
    TFT_eSPI *gfx, int16_t x1, int16_t y1, uint16_t w, uint16_t h,
    uint16_t outline, uint16_t activeFill, uint16_t inactiveFill, uint16_t textcolor,
    char *label, GFXfont *activeFont, GFXfont *inactiveFont, uint8_t textsize)
{
  _x1 = x1;
  _y1 = y1;
  _w = w;
  _h = h;
  _outlinecolor = outline;
  _activefill = activeFill;
  _inactivefill = inactiveFill;
  _textcolor = textcolor;
  _textsize = textsize;
  _activefont = activeFont;
  _inactivefont = inactiveFont;
  _gfx = gfx;
  strncpy(_label, label, 9);
}

// Adjust text datum and x, y deltas
void TFT_eSPI_Tab::setLabelDatum(int16_t x_delta, int16_t y_delta, uint8_t datum)
{
  _xd = x_delta;
  _yd = y_delta;
  _textdatum = datum;
}

void TFT_eSPI_Tab::drawTab(String long_name)
{
  uint16_t fill, bottomLineColor;
  uint8_t offset = _w / 12;

  // changing (in)active parameters
  if (currstate)
  {
    bottomLineColor = fill = _activefill;
    _gfx->setFreeFont(_activefont);
  }
  else
  {
    fill = _inactivefill;
    _gfx->setFreeFont(_inactivefont);
    bottomLineColor = _outlinecolor;
  }
  _gfx->fillTriangle(_x1, _y1 + _h, _x1 + offset, _y1, _x1 + offset, _y1 + _h, fill);
  _gfx->fillRect(_x1 + offset, _y1, _w - (2 * offset), _h, fill);
  _gfx->fillTriangle(_x1 + _w - offset, _y1, _x1 + _w, _y1 + _h, _x1 + _w - offset, _y1 + _h, fill);

  _gfx->drawLine(_x1, _y1 + _h, _x1 + offset, _y1, _outlinecolor);          // left side of tab
  _gfx->drawFastHLine(_x1 + offset, _y1, _w - (2 * offset), _outlinecolor); // top side of tab
  _gfx->drawLine(_x1 + _w - offset, _y1, _x1 + _w, _y1 + _h, _outlinecolor); // right side of tab
  _gfx->drawFastHLine(_x1, _y1 + _h, _w, bottomLineColor); //bottom line

  _gfx->setTextColor(_textcolor, fill);
  _gfx->setTextSize(_textsize);

  uint8_t tempdatum = _gfx->getTextDatum();
  _gfx->setTextDatum(_textdatum);
  uint16_t tempPadding = _gfx->getTextPadding();
  _gfx->setTextPadding(0);

  if (long_name == "")
    _gfx->drawString(_label, _x1 + (_w / 2) + _xd, _y1 + (_h / 2) - 4 + _yd);
  else
    _gfx->drawString(long_name, _x1 + (_w / 2) + _xd, _y1 + (_h / 2) - 4 + _yd);

  _gfx->setTextDatum(tempdatum);
  _gfx->setTextPadding(tempPadding);
}

bool TFT_eSPI_Tab::contains(int16_t x, int16_t y)
{
  return ((x >= _x1) && (x < (_x1 + _w)) &&
          (y >= _y1) && (y < (_y1 + _h)));
}

void TFT_eSPI_Tab::open(bool o){
  laststate = currstate;
  currstate = o;  
}

bool TFT_eSPI_Tab::isOpen(){
  return currstate;
}

void TFT_eSPI_Tab::press(bool p)
{
  laststate = currstate;
  currstate = p;
}

bool TFT_eSPI_Tab::isPressed() { return currstate; }
bool TFT_eSPI_Tab::justPressed() { return (currstate && !laststate); }
bool TFT_eSPI_Tab::justReleased() { return (!currstate && laststate); }
