#include "libtexproma_private.h"

void tpm_flip(tpm_mono_buf dst, tpm_mono_buf src) {
  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++)
      tpm_put_pixel(dst, (TP_WIDTH - 1) - x, y, tpm_get_pixel(src, x, y));
}

void tpm_rotate(tpm_mono_buf dst, tpm_mono_buf src) {
  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++)
      tpm_put_pixel(dst, y, x, tpm_get_pixel(src, x, y));
}

void tpm_twist(tpm_mono_buf dst, tpm_mono_buf src, float strength) {
  float s = constrain(strength, -1.0f, 1.0f) * 2.0f * M_PI / TP_WIDTH;

  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x++) {
      float xp = (float)(x - 128);
      float yp = (float)(y - 128);

      float r = sqrtf(xp * xp + yp * yp);
      
      int c;

      if (r <= 128.0f) {
        float ang = atan2(xp, yp) + (s * (128.0f - r));

        xp = sinf(ang) * r + 128.0f;
        yp = cosf(ang) * r + 128.0f;

        c = tpm_get_filtered_pixel(src, xp, yp);
      } else {
        c = tpm_get_pixel(src, x, y);
      }

      tpm_put_pixel(dst, x, y, c);
    }
  }
}

void tpm_move(tpm_mono_buf dst, tpm_mono_buf src, float move_x, float move_y) {
  int xo = constrain(move_x, -1.0f, 1.0f) * TP_WIDTH;
  int yo = constrain(move_y, -1.0f, 1.0f) * TP_HEIGHT;

  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++)
      tpm_put_pixel(dst, x, y, tpm_get_pixel(src, x + xo, y + yo));
}

void tpm_distort(tpm_mono_buf dst, tpm_mono_buf src,
                 tpm_mono_buf umap, tpm_mono_buf vmap, 
                 float ustrength, float vstrength)
{
  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x++) {
      float fu = (float)tpm_get_pixel(umap, x, y) * ustrength + (float)x;
      float fv = (float)tpm_get_pixel(vmap, x, y) * vstrength + (float)y;

      tpm_put_pixel(dst, x, y, tpm_get_filtered_pixel(src, fu, fv));
    }
  }
}
