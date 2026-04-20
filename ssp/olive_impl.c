#define OLIVECDEF
#define OLIVEC_IMPLEMENTATION
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#include <ssp/olive.c>
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

Olivec_Font ssp_olive_default_font(void)
{
	return olivec_default_font;
}
