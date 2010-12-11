#ifndef __LIBTEXPROMA_H__
#define __LIBTEXPROMA_H__

#include <stdint.h>

#define TP_WIDTH 256
#define TP_HEIGHT 256

typedef struct {
  uint8_t r, g, b;
} color_t;

typedef uint8_t *tpm_mono_buf;
typedef color_t *tpm_color_buf;

/* generate ops */
void tpm_plasma(tpm_mono_buf dst, uint8_t xsines, uint8_t ysines,
                float xphase, float yphase);
void tpm_light(tpm_mono_buf dst, uint8_t type, float radius);
void tpm_perlin_plasma(tpm_mono_buf dst, uint8_t step, uint32_t seed);

/* buffer ops */
void tpm_set(tpm_mono_buf dst, int value);
void tpm_add(tpm_mono_buf dst, tpm_mono_buf src1, tpm_mono_buf src2);
void tpm_mul(tpm_mono_buf dst, tpm_mono_buf src1, tpm_mono_buf src2);
void tpm_mix(tpm_mono_buf dst, tpm_mono_buf src1, tpm_mono_buf src2,
             int percent);
void tpm_max(tpm_mono_buf dst, tpm_mono_buf src1, tpm_mono_buf src2);
void tpm_shade(tpm_mono_buf dst, tpm_mono_buf src1, tpm_mono_buf src2);
void tpm_mix_map(tpm_mono_buf dst, tpm_mono_buf src1, tpm_mono_buf src2,
                 tpm_mono_buf map);

/* distortion ops */
void tpm_twist(tpm_mono_buf dst, tpm_mono_buf src, float strenght);
void tpm_move(tpm_mono_buf dst, tpm_mono_buf src, int xoffset, int yoffset);
void tpm_uvmap(tpm_mono_buf dst, tpm_mono_buf src,
               tpm_mono_buf umap, tpm_mono_buf vmap, 
               float ustrength, float vstrength);

#if 0
void tpm_colors_change_hsv(tpm_rgb_buf_t *dst, tpm_rgb_buf_t *src,
                           float hue, float saturation);
void tpm_colors_invert(tpm_mono_buf_t *dst, tpm_mono_buf_t *src);
void tpm_colors_sine_color(tpm_mono_buf_t *dst, tpm_mono_buf_t *src,
                           uint8_t sines);
void tpm_colors_contrast(tpm_mono_buf_t *dst, tpm_mono_buf_t *src,
                         uint8_t contrast);
void tpm_colors_colorize(tpm_rgb_buf_t *dst, tpm_mono_buf_t *src,
                         color_t c1, color_t c2);
void tpm_colors_average(tpm_mono_buf_t *dst, tpm_rgb_buf_t *src);
#endif

#endif
