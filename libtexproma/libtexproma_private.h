#ifndef __LIBTEXPROMA_PRIVATE_H__
#define __LIBTEXPROMA_PRIVATE_H__

#include <assert.h>
#include <math.h>
#include <stdint.h>
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
colori tpm_get_rgb_pixel(tpm_color_buf buf, int x, int y);
void tpm_put_rgb_pixel(tpm_color_buf buf, int x, int y, colori c);
int tpm_get_filtered_pixel(tpm_mono_buf buf, float x, float y);

uint32_t tpm_random(uint32_t *seed);
float tpm_bezier_interpolate(float s1, float p1, float p2, float t1, float t2);
void tpm_rgb_to_hsv(float r, float g, float b, float *h, float *s, float *v);
void tpm_hsv_to_rgb(float *r, float *g, float *b, float h, float s, float v);

#endif
