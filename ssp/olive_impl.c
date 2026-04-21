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

#include <od/graphics/fonts.h>

static int ssp_olive_od_advance(const font_t *font)
{
#if FONT_USE_WIDTH_TO_ADVANCE
	return font->width + 1;
#else
	return font->xadvance;
#endif
}

void ssp_olive_od_text_metrics(const char *text, int fontSize, int *width, int *height)
{
	int penX = 0;
	int minX = 0;
	int minY = 0;
	int maxX = 0;
	int maxY = 0;
	int hasGlyph = 0;

	if (width)
	{
		*width = 0;
	}
	if (height)
	{
		*height = 0;
	}

	if (!text || text[0] == '\0')
	{
		return;
	}

	for (const char *p = text; *p; ++p)
	{
		const font_t *font = font_lookup(fontSize, (uint8_t)*p);
		if (!font)
		{
			continue;
		}

		int x0 = penX + font->xoffset;
		int y0 = -font->yoffset - (font->height - 1);
		int x1 = x0 + font->width;
		int y1 = y0 + font->height;

		if (!hasGlyph)
		{
			minX = x0;
			minY = y0;
			maxX = x1;
			maxY = y1;
			hasGlyph = 1;
		}
		else
		{
			if (x0 < minX)
				minX = x0;
			if (y0 < minY)
				minY = y0;
			if (x1 > maxX)
				maxX = x1;
			if (y1 > maxY)
				maxY = y1;
		}

		penX += ssp_olive_od_advance(font);
	}

	if (hasGlyph)
	{
		if (width)
		{
			*width = maxX - minX;
		}
		if (height)
		{
			*height = maxY - minY;
		}
	}
}

void ssp_olive_od_text(Olivec_Canvas oc, const char *text, int x, int y, int fontSize, uint32_t color)
{
	int penX = 0;
	int textW = 0;
	int textH = 0;
	int minX = 0;
	int minY = 0;
	int haveOffset = 0;

	if (!text || text[0] == '\0')
	{
		return;
	}

	ssp_olive_od_text_metrics(text, fontSize, &textW, &textH);
	(void)textW;
	(void)textH;

	for (const char *p = text; *p; ++p)
	{
		const font_t *font = font_lookup(fontSize, (uint8_t)*p);
		if (!font)
		{
			continue;
		}

		int gx0 = penX + font->xoffset;
		int gy0 = -font->yoffset - (font->height - 1);
		if (!haveOffset)
		{
			minX = gx0;
			minY = gy0;
			haveOffset = 1;
		}
		else
		{
			if (gx0 < minX)
				minX = gx0;
			if (gy0 < minY)
				minY = gy0;
		}

		penX += ssp_olive_od_advance(font);
	}

	if (!haveOffset)
	{
		return;
	}

	penX = 0;
	for (const char *p = text; *p; ++p)
	{
		const font_t *font = font_lookup(fontSize, (uint8_t)*p);
		if (!font)
		{
			continue;
		}

		const uint8_t *bitmap = font->bitmap;
		int drawX = x + penX + font->xoffset - minX;
		int drawY = y - font->yoffset - (font->height - 1) - minY;

		for (int outJ = 0; outJ < font->height; ++outJ)
		{
			int srcJ = (font->height - 1) - outJ;
			const uint8_t *row = bitmap + srcJ * font->width;
			int py = drawY + outJ;
			if (py < 0 || py >= (int)oc.height)
			{
				continue;
			}

			for (int i = 0; i < font->width; ++i)
			{
				int px = drawX + i;
				if (px >= 0 && px < (int)oc.width && row[i])
				{
					olivec_blend_color(&OLIVEC_PIXEL(oc, px, py), color);
				}
			}
		}

		penX += ssp_olive_od_advance(font);
	}
}

Olivec_Font ssp_olive_default_font(void)
{
	return olivec_default_font;
}
