#include "libtexproma_private.h"

void tpm_hsv_modify(tpm_color_buf dst, tpm_color_buf src,
                    float hue, float sat)
{
  hue = constrain(hue, 0.0f, 1.0f);
  sat = constrain(sat, 0.0f, 1.0f);

  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x++) {
      float r = tpm_get_pixel(src[0], x, y);
      float g = tpm_get_pixel(src[1], x, y);
      float b = tpm_get_pixel(src[2], x, y);
      float h, s, v;

      tpm_rgb_to_hsv(r, g, b, &h, &s, &v);

      h += hue;

      if (h > 1.0f)
          h -= 1.0f;

      s *= sat;

      tpm_hsv_to_rgb(&r, &g, &b, h, s, v);

      tpm_put_pixel(dst[0], x, y, r);
      tpm_put_pixel(dst[1], x, y, g);
      tpm_put_pixel(dst[2], x, y, b);
    }
  }
}

void tpm_invert(tpm_mono_buf dst, tpm_mono_buf src) {
  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++)
      tpm_put_pixel(dst, x, y, 255 - tpm_get_pixel(src, x, y));
}

void tpm_sine_color(tpm_mono_buf dst, tpm_mono_buf src, unsigned cycles) {
  static int tab[256];

  cycles = constrain(cycles, 1, 32);

  const float di = cycles * 2.0f * M_PI / 255.0f;

  for (int i = 0; i < 256; i++)
      tab[i] = 0.5f * (1.0f - cosf(i * di)) * 255.0f;

  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++)
      tpm_put_pixel(dst, x, y, tab[tpm_get_pixel(src, x, y)]);
}


/*
 * @param factor - positive will brighten, negative will darken picture
 */
void tpm_brightness(tpm_mono_buf dst, tpm_mono_buf src, float factor) {
  factor = constrain(factor, -1.0f, 1.0f);

  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x++) {
      float p = tpm_get_pixel(src, x, y);

      tpm_put_pixel(dst, x, y, p * (1.0f + factor));
    }
  }
}

/*
 * @param constrast - contrast change
 */
void tpm_contrast(tpm_mono_buf dst, tpm_mono_buf src, float contrast) {
  contrast = constrain(contrast, -1.0f, 1.0f) + 1.0f;

  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x++) {
      float p = tpm_get_pixel(src, x, y);

      tpm_put_pixel(dst, x, y, (p - 128.0f) * contrast + 128.0f);
    }
  }
}

#define R(x) (((x) & 0xff0000) >> 16)
#define G(x) (((x) & 0x00ff00) >> 8)
#define B(x) ((x) & 0x0000ff)

void tpm_colorize(tpm_color_buf dst, tpm_mono_buf src, unsigned c1, unsigned c2)
{
  const float r = 1.0f / 255.0f;

  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x++) {
      float p = r * tpm_get_pixel(src, x, y);

      tpm_put_pixel(dst[0], x, y, lerp(R(c1), R(c2), p));
      tpm_put_pixel(dst[1], x, y, lerp(G(c1), G(c2), p));
      tpm_put_pixel(dst[2], x, y, lerp(B(c1), B(c2), p));
    }
  }
}

void tpm_grayscale(tpm_mono_buf dst, tpm_color_buf src) {
  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x++) {
      int r = tpm_get_pixel(src[0], x, y);
      int g = tpm_get_pixel(src[1], x, y);
      int b = tpm_get_pixel(src[2], x, y);
      tpm_put_pixel(dst, x, y, 0.2126f * r + 0.7152f * g + 0.0722f * b);
    }
  }
}
