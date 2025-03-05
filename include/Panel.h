#ifndef PANEL_H
#define PANEL_H

#include <list>
#include "DisplayElement.h"

class Panel
{
public:
  void addElement(DisplayElement dE);
  std::list<DisplayElement> displayElements;

private:
};

#endif
