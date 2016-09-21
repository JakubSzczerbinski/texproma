#include "libtexproma_private.h"

#if 0
void tpm_change_hsv(tpm_color_buf dst, tpm_color_buf src,
                    float hue, float sat)
{
  hue = constrain(hue, 0.0f, 1.0f);
  sat = constrain(sat, 0.0f, 1.0f);

  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x++) {
      colorf_t c;

      tp_reg_get_rgb_pixelf(rc_src, x, y, &c);

      float h, s, v;

      tp_rgb_to_hsv(c.r, c.g, c.b, &h, &s, &v);

      h += hue;

      if (h > 1.0f)
          h -= 1.0f;

      s *= sat;

      tpm_hsv_to_rgb(&c.r, &c.g, &c.b, h, s, v);

      tpm_put_rgb_pixelf(rc_dst, x, y, &c);
    }
  }
}
#endif

void tpm_invert(tpm_mono_buf dst, tpm_mono_buf src) {
  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++)
      tpm_put_pixel(dst, x, y, ~tpm_get_pixel(src, x, y));
}

void tpm_sine_color(tpm_mono_buf dst, tpm_mono_buf src, unsigned sine_cycles) {
  static int tab[256];

  sine_cycles = constrain(sine_cycles, 2, 32);

  for (int i = 0; i < 256; i++)
      tab[i] = 127 - (sinf(i / 256.0f * sine_cycles * M_2_PI) * 127.0f) + 127;

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

      p += (factor < 0) ? (-p * factor) : ((1.0f - p) * factor);

      tpm_put_pixel(dst, x, y, (int)p);
    }
  }
}

/*
 * @param constrast - contrast change
 */
void tpm_contrast(tpm_mono_buf dst, tpm_mono_buf src, float contrast) {
  contrast = constrain(contrast, 0.0f, 1.0f);

  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x++) {
      float p = tpm_get_pixel(src, x, y);

      tpm_put_pixel(dst, x, y, (p - 128.0f) * contrast + 128.0f);
    }
  }
}

void tpm_colorize(tpm_color_buf dst, tpm_mono_buf src, color_t c1, color_t c2) {
  colori d = { c2.r - c1.r, c2.g - c1.g, c2.b - c1.b };

  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x++) {
      int p = tpm_get_pixel(src, x, y);

      colori c;

      c.r = ((d.r * p) >> 8) + c1.r;
      c.g = ((d.g * p) >> 8) + c1.g;
      c.b = ((d.b * p) >> 8) + c2.b;

      tpm_put_color_pixel(dst, x, y, c);
    }
  }
}

void tpm_grayscale(tpm_mono_buf dst, tpm_color_buf src) {
  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x++) {
      colori c = tpm_get_color_pixel(src, x, y);
      tpm_put_pixel(dst, x, y, 0.2126f * c.r + 0.7152f * c.g + 0.0722f * c.b);
    }
  }
}
