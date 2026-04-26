#pragma once

namespace percussa
{
  namespace ui
  {
    class Rect
    {
    public:
      Rect() : mX(0), mY(0), mW(0), mH(0)
      {
      }

      Rect(int xValue, int yValue, int wValue, int hValue) : mX(xValue), mY(yValue), mW(wValue), mH(hValue)
      {
      }

      int x() const
      {
        return mX;
      }

      int y() const
      {
        return mY;
      }

      int w() const
      {
        return mW;
      }

      int h() const
      {
        return mH;
      }

    private:
      int mX;
      int mY;
      int mW;
      int mH;
    };
  } // namespace ui
} // namespace percussa