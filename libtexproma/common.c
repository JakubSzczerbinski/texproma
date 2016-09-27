#include "libtexproma_private.h"

int tpm_get_pixel(tpm_mono_buf buf, int x, int y) {
  x &= (TP_WIDTH - 1);
  y &= (TP_HEIGHT - 1);

  return buf[y * TP_WIDTH + x];
}

void tpm_put_pixel(tpm_mono_buf buf, int x, int y, int c) {
  x &= (TP_WIDTH - 1);
  y &= (TP_HEIGHT - 1);

  buf[y * TP_WIDTH + x] = constrain(c, 0, 255);
}

/* https://en.wikipedia.org/wiki/Bilinear_filtering */
int tpm_get_filtered_pixel(tpm_mono_buf buf, float x, float y) {
  float u = floorf(x), v = floorf(y);
  int xi = u, yi = v;

  u = x - u;
  v = y - v;

  float p1 = tpm_get_pixel(buf, xi    , yi    );
  float p2 = tpm_get_pixel(buf, xi + 1, yi    );
  float p3 = tpm_get_pixel(buf, xi    , yi + 1);
  float p4 = tpm_get_pixel(buf, xi + 1, yi + 1);

  return ((p1 * (1.0f - u) + p2 * u) * (1.0f - v) +
          (p3 * (1.0f - u) + p4 * u) * v);
}

uint32_t tpm_random(uint32_t *seed) {
  uint32_t s = *seed;
  s ^= s << 13;
  s ^= s >> 17;
  s ^= s << 5;
  *seed = s;
  return s;
}

float tpm_bezier_interpolate(float s1, float p1, float p2, float t1, float t2) {
  float s2, s3, h1, h2, h3, h4;

  s2 = s1 * s1;
  s3 = s2 * s1;
  h2 = -2.0f * s3 + 3.0f * s2;
  h1 = -h2 + 1.0f;
  h4 = s3 - s2;
  h3 = h4 - s2 + s1;

  return h1 * p1 + h2 * p2 + h3 * t1 + h4 * t2;
}

void tpm_rgb_to_hsv(float r, float g, float b, float *h, float *s, float *v) {
  float t1 = r;
  float t2 = g;
  float t3 = b;
  float tt;

  if (t1 > t2) {
    tt = t1; t1 = t2; t2 = tt;
  }

  if (t1 > t3) {
    tt = t1; t1 = t3; t3 = tt;
  }

  if (t2 > t3) {
    tt = t2; t2 = t3; t3 = tt;
  }

  float max = t3;
  float min = t1;
  float diff = max - min;

  *v = max;
  *s = (max == 0.0f) ? 0.0f : (diff / max);

  if (*s != 0.0f) {
    if (r == max) {
      *h = (g - b) / diff;
    } else {
      if (g == max) {
        *h = 2.0 + (b - r) / diff;
      } else {
        *h = 4.0 + (r - g) / diff;
      }
    }

    *h /= 6.0f;

    if (*h < 0.0f)
        *h += 1.0f;
  } else {
    *h = 0.0f;
  }

}

void tpm_hsv_to_rgb(float *r, float *g, float *b, float h, float s, float v) {
  if (s == 0.0f) {
    *r = v;
    *g = v;
    *b = v;
  } else {
    if (h == 1.0f)
        h = 0.0f;

    h *= 6.0f;

    float hi = floorf(h);
    float hf = h - hi;

    float p = v * (1.0f - s);
    float q = v * (1.0f - s * hf);
    float t = v * (1.0f - s * (1.0f - hf));

    switch ((int)hi) {
      case 0:
        *r = v;
        *g = t;
        *b = p;
        break;

      case 1:
        *r = q;
        *g = v;
        *b = p;
        break;

      case 2:
        *r = p;
        *g = v;
        *b = t;
        break;

      case 3:
        *r = p;
        *g = q;
        *b = v;
        break;

      case 4:
        *r = t;
        *g = p;
        *b = v;
        break;

      case 5:
        *r = v;
        *g = p;
        *b = q;
        break;
    }
  }
}
