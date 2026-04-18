#include <hal/fft.h>
#include <hal/heap.h>

#include <string.h>

typedef struct
{
  int n;
} stub_fft_handle_t;

handle_rfft_t RFFT_allocate(int n)
{
  stub_fft_handle_t *handle = (stub_fft_handle_t *)Heap_calloc(1, sizeof(stub_fft_handle_t));
  handle->n = n;
  return (handle_rfft_t)handle;
}

void RFFT_destroy(handle_rfft_t handle)
{
  Heap_free(handle);
}

void RFFT_forward(complex_float_t *dst, float *src, handle_rfft_t handle)
{
  stub_fft_handle_t *h = (stub_fft_handle_t *)handle;
  int bins = h->n / 2 + 1;
  memset(dst, 0, bins * sizeof(complex_float_t));

  // Preserve DC component approximately in no-FFTW mode.
  float sum = 0.0f;
  for (int i = 0; i < h->n; i++)
  {
    sum += src[i];
  }
  dst[0].r = sum;
}

void RFFT_inverse(float *dst, complex_float_t *src, handle_rfft_t handle)
{
  (void)src;
  stub_fft_handle_t *h = (stub_fft_handle_t *)handle;
  memset(dst, 0, h->n * sizeof(float));
}

handle_cfft_t CFFT_allocate(int n)
{
  stub_fft_handle_t *handle = (stub_fft_handle_t *)Heap_calloc(1, sizeof(stub_fft_handle_t));
  handle->n = n;
  return (handle_cfft_t)handle;
}

void CFFT_destroy(handle_cfft_t handle)
{
  Heap_free(handle);
}

void CFFT_forward(complex_float_t *dst, complex_float_t *src, handle_cfft_t handle)
{
  stub_fft_handle_t *h = (stub_fft_handle_t *)handle;
  memcpy(dst, src, h->n * sizeof(complex_float_t));
}

void CFFT_inverse(complex_float_t *dst, complex_float_t *src, handle_cfft_t handle)
{
  stub_fft_handle_t *h = (stub_fft_handle_t *)handle;
  float scale = h->n > 0 ? 1.0f / h->n : 1.0f;
  for (int i = 0; i < h->n; i++)
  {
    dst[i].r = src[i].r * scale;
    dst[i].i = src[i].i * scale;
  }
}
