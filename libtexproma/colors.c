#include "libtxtproc_private.h"

/**
 * @param rc_dst  destination (RGB register number)
 * @param rc_src  source (RGB register number)
 * @param fcu_hue hue change (unsigned clamped float [0.0f, 1.0f])
 * @param fcu_sat saturation change (unsigned clamped float [0.0f, 1.0f])
 */

void tp_op_colors_change_hsv(uint8_t rc_dst, uint8_t rc_src, float fcu_hue, float fcu_sat)
{
  int16_t x, y;

  for (y = 0; y < TP_HEIGHT; y++) {
    for (x = 0; x < TP_WIDTH; x++) {
      colorf_t c;

      tp_reg_get_rgb_pixelf(rc_src, x, y, &c);

      float h, s, v;

      tp_rgb_to_hsv(c.r, c.g, c.b, &h, &s, &v);

      h += fcu_hue;

      if (h > 1.0f)
          h -= 1.0f;

      s *= fcu_sat;

      tp_hsv_to_rgb(&c.r, &c.g, &c.b, h, s, v);

      tp_reg_put_rgb_pixelf(rc_dst, x, y, &c);
    }
  }
}

/**
 * @param r_dst   destination (register number)
 * @param r_src   source (register number)
 */

void tp_op_colors_invert(uint8_t r_dst, uint8_t r_src)
{
  int16_t x, y;

  for (y = 0; y < TP_HEIGHT; y++)
    for (x = 0; x < TP_WIDTH; x++)
      tp_reg_put_pixel(r_dst, x, y, ~tp_reg_get_pixel(r_src, x, y));
}

/**
 * @param r_dst   destination (register number)
 * @param r_src   source (register number)
 * @param bu_sines  number of sine cycles (unsigned byte [2, 32])
 */

static int16_t tp_tab[256];

void tp_op_colors_sine_color(uint8_t r_dst, uint8_t r_src, uint8_t bu_sines)
{
  int i;

  for (i = 0; i < 256; i++)
      tp_tab[i] = 127 - (int16_t)(sinf(i / 256.0f * bu_sines * M_2_PI) * 127.0f) + 127;

  int16_t x, y;

  for (y = 0; y < TP_HEIGHT; y++)
    for (x = 0; x < TP_WIDTH; x++)
      tp_reg_put_pixel(r_dst, x, y, tp_tab[tp_reg_get_pixel(r_src, x, y)]);
}

/**
 * @param r_dst     destination (register number)
 * @param r_src     source (register number)
 * @param ics_amount  brightness factor - positive will brighten, negative will darken picture (clamped signed fixed 8.8 [-1.0, 1.0])
 */

void tp_op_colors_bright(uint8_t r_dst, uint8_t r_src, int16_t ics_amount)
{
  int16_t x, y;

  for (y = 0; y < TP_HEIGHT; y++) {
    for (x = 0; x < TP_WIDTH; x++) {
      int16_t p = tp_reg_get_pixel(r_src, x, y);

      p += (ics_amount < 0) ? ((-p * ics_amount) >> 8) : (((255 - p) * ics_amount) >> 8);

      tp_reg_put_pixel(r_dst, x, y, p);
    }
  }
}

/**
 * @param r_dst     destination (register number)
 * @param r_src     source (register number)
 * @param bu_constrast  contrast change (unsigned byte)
 */

void tp_op_colors_contrast(uint8_t r_dst, uint8_t r_src, uint8_t bu_contrast)
{
  int16_t cont = 127 + (bu_contrast >= 0) ? (bu_contrast * 7) : (bu_contrast * 120 / 127);

  int16_t x, y;

  for (y = 0; y < TP_HEIGHT; y++) {
    for (x = 0; x < TP_WIDTH; x++) {
      int16_t p = tp_reg_get_pixel(r_src, x, y) - 128;

      p = (bu_contrast >= 0) ? ((p * cont) >> 7) : ((p << 7) / cont);

      tp_reg_put_pixel(r_dst, x, y, p + 128);
    }
  }
}

/**
 * @param r_dst   destination (RGB register number)
 * @param r_src   source (register number)
 * @param c_c1    first color (RGB color)
 * @param c_c2    second color (RGB color)
 */

void tp_op_colors_colorize(uint8_t dst, uint8_t src, color_t c_c1, color_t c_c2)
{
  colori_t d = { c_c2.r - c_c1.r, c_c2.g - c_c1.g, c_c2.b - c_c1.b };

  int16_t x, y;

  for (y = 0; y < TP_HEIGHT; y++) {
    for (x = 0; x < TP_WIDTH; x++) {
      int16_t p = tp_reg_get_pixel(src, x, y);

      colori_t c;

      c.r = ((d.r * p) >> 8) + c_c1.r;
      c.g = ((d.g * p) >> 8) + c_c1.g;
      c.b = ((d.b * p) >> 8) + c_c2.b;

      tp_reg_put_rgb_pixel(dst, x, y, &c);
    }
  }
}

/**
 * @param r_dst   destination (register number)
 * @param r_src   source (RGB register number)
 */

void tp_op_colors_average(uint8_t r_dst, uint8_t r_src)
{
  int16_t x = 0, y = 0;

  for (y = 0; y < TP_HEIGHT; y++) {
    for (x = 0; x < TP_WIDTH; x++) {
      colori_t c;
    
      tp_reg_get_rgb_pixel(r_src, x, y, &c);
      tp_reg_put_pixel(r_dst, x, y, (c.r + c.g + c.b) / 3);
    }
  }
}
