#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

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
void olivec_fill(Olivec_Canvas oc, uint32_t color);
void olivec_rect(Olivec_Canvas oc, int x, int y, int w, int h, uint32_t color);
void olivec_frame(Olivec_Canvas oc, int x, int y, int w, int h, size_t thiccness, uint32_t color);
void olivec_circle(Olivec_Canvas oc, int cx, int cy, int r, uint32_t color);
void olivec_text(Olivec_Canvas oc, const char *text, int x, int y, Olivec_Font font, size_t size, uint32_t color);
Olivec_Font ssp_olive_default_font(void);

#ifdef __cplusplus
}
#endif

#define SSP_RGBA(r, g, b, a) ((((uint32_t)(r)&0xFF) << 24) | (((uint32_t)(g)&0xFF) << 16) | (((uint32_t)(b)&0xFF) << 8) | ((uint32_t)(a)&0xFF))
#define SSP_OLIVE_RGBA(r, g, b, a) ((((uint32_t)(r)&0xFF) << 0) | (((uint32_t)(g)&0xFF) << 8) | (((uint32_t)(b)&0xFF) << 16) | (((uint32_t)(a)&0xFF) << 24))
