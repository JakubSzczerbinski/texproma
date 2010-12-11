#include "libtexproma_private.h"

void tpm_set(tpm_mono_buf dst, int value) {
  memset(dst, value, TP_WIDTH * TP_HEIGHT);
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
