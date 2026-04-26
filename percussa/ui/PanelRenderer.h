#pragma once

#include <string>
#include <stdint.h>

#include <vector>

namespace percussa
{
  namespace panel
  {
    class Panel;
  }

  namespace ui
  {
    class RenderedPanel
    {
    public:
      RenderedPanel() : mWidth(0), mHeight(0)
      {
      }

      RenderedPanel(int width, int height)
          : mWidth(width), mHeight(height), mPixels((size_t)width * (size_t)height)
      {
      }

      int width() const
      {
        return mWidth;
      }

      int height() const
      {
        return mHeight;
      }

      uint32_t *data()
      {
        return mPixels.empty() ? 0 : &mPixels[0];
      }

      const uint32_t *data() const
      {
        return mPixels.empty() ? 0 : &mPixels[0];
      }

      std::vector<uint32_t> &pixels()
      {
        return mPixels;
      }

      const std::vector<uint32_t> &pixels() const
      {
        return mPixels;
      }

    private:
      int mWidth;
      int mHeight;
      std::vector<uint32_t> mPixels;
    };

    class PanelRenderer
    {
    public:
      RenderedPanel render(const panel::Panel &panel) const;
    };
  } // namespace ui
} // namespace percussa