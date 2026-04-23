#pragma once

namespace percussa
{
  namespace ui
  {
    struct Rect
    {
      Rect() : x(0), y(0), w(0), h(0)
      {
      }

      Rect(int xValue, int yValue, int wValue, int hValue) :
        x(xValue),
        y(yValue),
        w(wValue),
        h(hValue)
      {
      }

      int x;
      int y;
      int w;
      int h;
    };
  }
}