#include "libtexproma_private.h"

void tpm_insert(tpm_color_buf dst, tpm_mono_buf src, unsigned n) {
  n = constrain(n, 0, 2);
  memcpy(dst[n], src, TP_WIDTH * TP_HEIGHT);
}

void tpm_extract(tpm_color_buf src, tpm_mono_buf dst, unsigned n) {
  n = constrain(n, 0, 2);
  memcpy(dst, src[n], TP_WIDTH * TP_HEIGHT);
}

void tpm_color(tpm_color_buf dst, tpm_mono_buf src) {
  memcpy(dst[0], src, TP_WIDTH * TP_HEIGHT);
  memcpy(dst[1], src, TP_WIDTH * TP_HEIGHT);
  memcpy(dst[2], src, TP_WIDTH * TP_HEIGHT);
}

void tpm_explode(tpm_mono_buf r, tpm_mono_buf g, tpm_mono_buf b,
                 tpm_color_buf c) {
  for (int i = 0; i < TP_WIDTH * TP_HEIGHT; i++) {
    r[i] = c[0][i];
    g[i] = c[1][i];
    b[i] = c[2][i];
  }
}

void tpm_implode(tpm_color_buf c, 
                 tpm_mono_buf r, tpm_mono_buf g, tpm_mono_buf b) {
  for (int i = 0; i < TP_WIDTH * TP_HEIGHT; i++) {
    c[0][i] = r[i];
    c[1][i] = g[i];
    c[2][i] = b[i];
  }
}

void tpm_add(tpm_mono_buf dst, tpm_mono_buf src1, tpm_mono_buf src2) {
  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++)
      tpm_put_pixel(dst, x, y, (tpm_get_pixel(src1, x, y) +
                                tpm_get_pixel(src2, x, y)));
}

void tpm_mul(tpm_mono_buf dst, tpm_mono_buf src1, tpm_mono_buf src2) {
  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++)
      tpm_put_pixel(dst, x, y, (tpm_get_pixel(src1, x, y) *
                                tpm_get_pixel(src2, x, y)) >> 8);
}

void tpm_mix(tpm_mono_buf dst, tpm_mono_buf src1, tpm_mono_buf src2,
             int percent)
{
  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++)
      tpm_put_pixel(dst, x, y, 
                    ((tpm_get_pixel(src1, x, y) * percent) >> 8) +
                    ((tpm_get_pixel(src2, x, y) * (256 - percent)) >> 8));
}

void tpm_max(tpm_mono_buf dst, tpm_mono_buf src1, tpm_mono_buf src2) {
  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++) {
      int a = tpm_get_pixel(src1, x, y);
      int b = tpm_get_pixel(src2, x, y);
      tpm_put_pixel(dst, x, y, (a > b) ? a : b);
    }
}

void tpm_shade(tpm_mono_buf dst, tpm_mono_buf src1, tpm_mono_buf src2) {
  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++) {
      int v = tpm_get_pixel(src1, x, y);
      int d = tpm_get_pixel(src2, x, y);
      int p = (v < 128) ? ((d * v) >> 7) : ((((255 - d) * (v - 128)) >> 7) + d);

      tpm_put_pixel(dst, x, y, p);
    }
}

void tpm_mix_map(tpm_mono_buf dst, tpm_mono_buf src1, tpm_mono_buf src2,
                 tpm_mono_buf map)
{
  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++) {
      int percent = tpm_get_pixel(map, x, y);

      tpm_put_pixel(dst, x, y,
                    ((tpm_get_pixel(src1, x, y) * percent) >> 8) +
                    ((tpm_get_pixel(src2, x, y) * (256 - percent)) >> 8));
    }
}
