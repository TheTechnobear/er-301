#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// olive.c cannot be included directly from C++ because it uses C99 non-trivial
// designated initializers (the font glyph table). These declarations mirror the
// olive.c public API; update them if the olive API changes.

// if libs/olive_c/olive.c  changes, you will need to update this to reflect changes
typedef struct
{
  uint32_t *pixels;
  size_t width;
  size_t height;
  size_t stride;
} Olivec_Canvas;

typedef struct
{
  size_t width;
  size_t height;
  const char *glyphs;
} Olivec_Font;

Olivec_Canvas olivec_canvas(uint32_t *pixels, size_t width, size_t height, size_t stride);
Olivec_Canvas olivec_subcanvas(Olivec_Canvas oc, int x, int y, int w, int h);
void olivec_fill(Olivec_Canvas oc, uint32_t color);
void olivec_rect(Olivec_Canvas oc, int x, int y, int w, int h, uint32_t color);
void olivec_frame(Olivec_Canvas oc, int x, int y, int w, int h, size_t thiccness, uint32_t color);
void olivec_circle(Olivec_Canvas oc, int cx, int cy, int r, uint32_t color);
void olivec_ellipse(Olivec_Canvas oc, int cx, int cy, int rx, int ry, uint32_t color);
void olivec_line(Olivec_Canvas oc, int x1, int y1, int x2, int y2, uint32_t color);
void olivec_triangle(Olivec_Canvas oc, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
void olivec_text(Olivec_Canvas oc, const char *text, int x, int y, Olivec_Font font, size_t size, uint32_t color);
void olivec_sprite_blend(Olivec_Canvas oc, int x, int y, int w, int h, Olivec_Canvas sprite);
void olivec_sprite_copy(Olivec_Canvas oc, int x, int y, int w, int h, Olivec_Canvas sprite);

// this in defined in olive_impl.c, and returns olivec_default_font, if this changes it would need updating
Olivec_Font ssp_olive_default_font(void);

// Draw OD bitmap fonts into an Olive canvas using top-left placement.
void ssp_olive_od_text(Olivec_Canvas oc, const char *text, int x, int y, int fontSize, uint32_t color);

// Return text bounds for ssp_olive_od_text() top-left placement.
void ssp_olive_od_text_metrics(const char *text, int fontSize, int *width, int *height);

// OLIVEC_RGBA is trapped inside OLIVEC_IMPLEMENTATION in olive.c (should be in the header section).
// SSP_RGBA provides the equivalent for use in C++ application code.
// Pixel format: 0xAABBGGRR matching SDL_PIXELFORMAT_ABGR8888.

// copy of OLIVEC_RGBA from libs/olive_c/olive.c , if this changes, resync the macro below
#define OLIVEC_RGBA(r, g, b, a) ((((r)&0xFF)<<(8*0)) | (((g)&0xFF)<<(8*1)) | (((b)&0xFF)<<(8*2)) | (((a)&0xFF)<<(8*3)))


#ifdef __cplusplus
}
#endif

// a local abstraction, just in case 
#define SSP_RGBA(r, g, b, a) (OLIVEC_RGBA(r,g,b,a))