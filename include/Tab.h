#ifndef TFT_TAB
#include <TFT_eSPI.h>
#define TFT_TAB

/***************************************************************************************
// The following Tab class has been ported over from the Adafruit_GFX library so
// should be compatible.
// A slightly different implementation in this TFT_eSPI library allows the Tab
// legends to be in any font, allow longer labels and to adjust text positioning
// within Tab
***************************************************************************************/

class TFT_eSPI_Tab
{
public:
  TFT_eSPI_Tab(void);
  // "Classic" initTab() uses centre & size
  void initTab(TFT_eSPI *gfx, int16_t x, int16_t y,
               uint16_t w, uint16_t h, uint16_t outline, uint16_t activeFill,
               uint16_t inactiveFill, uint16_t textcolor, char *label,
               GFXfont *activeFont, GFXfont *inactiveFont, 
               uint8_t textsize);

  // New/alt initTab() uses upper-left corner & size
  void initTabUL(TFT_eSPI *gfx, int16_t x1, int16_t y1,
                 uint16_t w, uint16_t h, uint16_t outline, uint16_t activeFill,
                 uint16_t inactiveFill, uint16_t textcolor, char *label, 
                 GFXfont *activeFont, GFXfont *inactiveFont,
                 uint8_t textsize);

  // Adjust text datum and x, y deltas
  void setLabelDatum(int16_t x_delta, int16_t y_delta, uint8_t datum = MC_DATUM);

  void drawTab(String long_name = "");
  bool contains(int16_t x, int16_t y);

  void open(bool o);
  bool isOpen();

  void press(bool p);
  bool isPressed();
  bool justPressed();
  bool justReleased();

private:
  TFT_eSPI *_gfx;
  int16_t _x1, _y1;              // Coordinates of top-left corner of Tab
  int16_t _xd, _yd;              // Tab text datum offsets (wrt centre of Tab)
  uint16_t _w, _h;               // Width and height of Tab
  uint8_t _textsize, _textdatum; // Text size multiplier and text datum for Tab
  uint16_t _outlinecolor, _activefill, _inactivefill, _textcolor;
  GFXfont *_activefont, *_inactivefont;
  char _label[10]; // Tab text is 9 chars maximum unless long_name used

  bool currstate, laststate; // Tab states
};

#endif
