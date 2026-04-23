#define OLIVECDEF
#define OLIVEC_IMPLEMENTATION
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#include <libs/olive_c/olive.c>
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

Olivec_Font percussa_olive_default_font(void)
{
  return olivec_default_font;
}

void percussa_olive_text_metrics(const char *text, int fontSize, int *width, int *height)
{
  Olivec_Font font = percussa_olive_default_font();
  int glyphWidth = (int)(font.width * (size_t)fontSize);
  int glyphHeight = (int)(font.height * (size_t)fontSize);
  int count = 0;

  if (width)
  {
    *width = 0;
  }
  if (height)
  {
    *height = 0;
  }
  if (!text)
  {
    return;
  }

  while (text[count])
  {
    count++;
  }

  if (width)
  {
    *width = glyphWidth * count;
  }
  if (height)
  {
    *height = glyphHeight;
  }
}