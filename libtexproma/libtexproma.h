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

void tpm_explode(tpm_mono_buf r, tpm_mono_buf g, tpm_mono_buf b,
                 tpm_color_buf c);
void tpm_implode(tpm_color_buf c, 
                 tpm_mono_buf r, tpm_mono_buf g, tpm_mono_buf b);
void tpm_mono_buf_save(tpm_mono_buf src, char *filename);
void tpm_color_buf_save(tpm_color_buf src, char *filename);

/* generate ops */
void tpm_sine(tpm_mono_buf dst, float amplitude);
void tpm_noise(tpm_mono_buf dst, unsigned step, unsigned seed);
void tpm_plasma(tpm_mono_buf dst);
void tpm_light(tpm_mono_buf dst, unsigned type, float radius);
void tpm_perlin_noise(tpm_mono_buf dst, unsigned seed);

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

/* transformation ops */
void tpm_repeat(tpm_mono_buf dst, tpm_mono_buf src, unsigned nx, unsigned ny);
void tpm_flip(tpm_mono_buf dst, tpm_mono_buf src);
void tpm_rotate(tpm_mono_buf dst, tpm_mono_buf src);
void tpm_twist(tpm_mono_buf dst, tpm_mono_buf src, float strenght);
void tpm_move(tpm_mono_buf dst, tpm_mono_buf src, float move_x, float move_y);
void tpm_distort(tpm_mono_buf dst, tpm_mono_buf src,
                 tpm_mono_buf umap, tpm_mono_buf vmap, 
                 float ustrength, float vstrength);

/* color ops */
void tpm_invert(tpm_mono_buf dst, tpm_mono_buf src);
void tpm_hsv_modify(tpm_color_buf dst, tpm_color_buf src,
                    float hue, float sat);
void tpm_sine_color(tpm_mono_buf dst, tpm_mono_buf src, unsigned cycles);
void tpm_brightness(tpm_mono_buf dst, tpm_mono_buf src, float factor);
void tpm_contrast(tpm_mono_buf dst, tpm_mono_buf src, float contrast);
void tpm_colorize(tpm_color_buf dst, tpm_mono_buf src,
                  unsigned c1, unsigned c2);
void tpm_grayscale(tpm_mono_buf dst, tpm_color_buf src);

/* filter ops */
void tpm_blur_3x3(tpm_mono_buf dst, tpm_mono_buf src);
void tpm_blur_5x5(tpm_mono_buf dst, tpm_mono_buf src);
void tpm_gaussian_3x3(tpm_mono_buf dst, tpm_mono_buf src);
void tpm_gaussian_5x5(tpm_mono_buf dst, tpm_mono_buf src);
void tpm_sharpen(tpm_mono_buf dst, tpm_mono_buf src);
void tpm_emboss(tpm_mono_buf dst, tpm_mono_buf src);
void tpm_edges(tpm_mono_buf dst, tpm_mono_buf src);
void tpm_median_3x3(tpm_mono_buf dst, tpm_mono_buf src);
void tpm_median_5x5(tpm_mono_buf dst, tpm_mono_buf src);

#endif
