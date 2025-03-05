#ifndef CUSTOM_BUTTONS_H
#define CUSTOM_BUTTONS_H
#include <TFT_eSPI.h>
#include <vector>
#include "Utility.h"

struct Point
{
  int16_t x;
  int16_t y;
};

class CustomButton
{
public:
  CustomButton(void);
  // "Classic" initButton() uses centre & size
  virtual void initButton(TFT_eSPI *gfx, int16_t x, int16_t y,
                  uint16_t w, uint16_t h, uint16_t outline, uint16_t fill,
                  uint16_t textcolor, String label, uint8_t textsize);

  // New/alt initButton() uses upper-left corner & size
  void initButtonUL(TFT_eSPI *gfx, int16_t x1, int16_t y1,
                    uint16_t w, uint16_t h, uint16_t outline, uint16_t fill,
                    uint16_t textcolor, String label, uint8_t textsize);

  // Adjust text datum and x, y deltas
  void setLabelDatum(int16_t x_delta, int16_t y_delta, uint8_t datum = MC_DATUM);

  virtual void drawButton(bool inverted = false, String long_name = "");
  virtual bool contains(int16_t x, int16_t y);
  void press(bool p);
  bool isPressed();
  bool justPressed();
  bool justReleased();

protected:
  TFT_eSPI *_gfx;
  int16_t _x, _y;              // Coordinates of top-left corner of button
  int16_t _xd, _yd;              // Button text datum offsets (wrt centre of button)
  uint16_t _w, _h;               // Width and height of button
  uint8_t _textsize, _textdatum; // Text size multiplier and text datum for button
  uint16_t _outlinecolor, _fillcolor, _textcolor;
  bool currstate, laststate; // Button states
  
private:
  //char _label[10]; // Button text is 9 chars maximum unless long_name used
  String _label;
};

class IncrementButton : public CustomButton
{
public:
  void initButton(TFT_eSPI *gfx, int16_t x, int16_t y,
                  uint16_t w, uint16_t h, uint16_t outline, uint16_t fill,
                  uint16_t textcolor, bool positive);
  void drawButton(bool inverted = false, String long_name = "");
  bool contains(int16_t x, int16_t y);

protected:
  bool _positive;
  Point _v1, _v2, _v3;
  
};

#endif
