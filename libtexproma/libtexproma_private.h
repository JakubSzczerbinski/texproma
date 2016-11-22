#ifndef __LIBTEXPROMA_PRIVATE_H__
#define __LIBTEXPROMA_PRIVATE_H__

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libtexproma.h"

#define TP_WIDTH 256
#define TP_HEIGHT 256

typedef struct {
  int r, g, b;
} colori;

typedef struct {
  float r, g, b;
} colorf;

int tpm_get_pixel(tpm_mono_buf buf, int x, int y);
void tpm_put_pixel(tpm_mono_buf buf, int x, int y, int c);
int tpm_get_filtered_pixel(tpm_mono_buf buf, float x, float y);

uint32_t tpm_random(uint32_t *seed);
float tpm_bezier_interpolate(float s1, float p1, float p2, float t1, float t2);
void tpm_rgb_to_hsv(float r, float g, float b, float *h, float *s, float *v);
void tpm_hsv_to_rgb(float *r, float *g, float *b, float h, float s, float v);

#define constrain(x, min, max) ({               \
    typeof(x) _x = x;                           \
    typeof(x) _min = min;                       \
    typeof(x) _max = max;                       \
    if (_x < _min) _x = _min;                   \
    else if (x > max) _x = _max;                \
    _x;                                         \
  })

#define lerp(x0, x1, s) ({                      \
    float _x0 = x0, _x1 = x1;                   \
    (typeof(x0))(_x0 * (1.0f - s) + _x1 * s);   \
  })

static inline float smoothstep(float x) {
  return x * x * (3 - 2 * x);
}

static inline float smootherstep(float x) {
  return x * x * x * (x * (x * 6 - 15) + 10);
}

#endif
