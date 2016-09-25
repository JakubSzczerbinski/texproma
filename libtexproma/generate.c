#include "libtexproma_private.h"

void tpm_sine(tpm_mono_buf dst, unsigned sines, unsigned n, float amplitude) {
  sines = constrain(sines, 1, 100);
  amplitude = constrain(amplitude, 0.0, (float)n);

  const float dy = 2.0f * M_PI / TP_HEIGHT;

  for (int y = 0; y < TP_HEIGHT; y++) {
    int v = (amplitude / sines) * sinf(sines * y * dy) * (float)TP_WIDTH;
    for (int x = 0; x < TP_WIDTH; x++)
      tpm_put_pixel(dst, x, y, (n * x + v) & 255);
  }
}

void tpm_plasma(tpm_mono_buf dst, unsigned xsines, unsigned ysines) {
  xsines = constrain(xsines, 1, 100);
  ysines = constrain(ysines, 1, 100);

  const float dx = xsines * 2.0f * M_PI / TP_WIDTH;
  const float dy = ysines * 2.0f * M_PI / TP_HEIGHT;

  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x++) {
      float v = 0.5f * (sinf(y * dy) + sinf(x * dx)) + 1.0f;
      tpm_put_pixel(dst, x, y, constrain(lroundf(v * 128.0f), 0, 255));
    }
  }
}

void tpm_light(tpm_mono_buf dst, unsigned type, float radius) {
  type = constrain(type, 0, 1);
  radius = constrain(radius, 0.01f, 2.0f);

  float s = 3.0f - radius;

  for (int y = 0; y < TP_HEIGHT; y++) {
    int y2 = (y - TP_HEIGHT / 2) * (y - TP_HEIGHT / 2);

    for (int x = 0; x < TP_WIDTH; x++) {
      int x2 = (x - TP_WIDTH / 2) * (x - TP_WIDTH / 2);

      float r = sqrtf(x2 + y2) * s;
      int v;

      if (type == 0) {
        v = 255 - (int)r;
      } else {
        if (type == 1) {
          if (r == 0.0)
              r = 0.01;

          v = (int)(255.0f * log(r / 128.0f) / log(0.1f));
        } else {
          v = (int)(259.75f * exp(-r * r / 16384.0f) - 4.75f);
        }
      }

      tpm_put_pixel(dst, x, y, v);
    }
  }
}

void tpm_noise(tpm_mono_buf dst, unsigned step, unsigned seed) {
  step = constrain(step, 0, 7);

  const unsigned n = 1 << step;
  const float ds = 1.0f / n;
  
  for (int y = 0; y < TP_HEIGHT; y += n)
    for (int x = 0; x < TP_WIDTH; x += n)
      tpm_put_pixel(dst, x, y, tpm_random(&seed) & 255);

  if (step == 0)
    return;

  /* interpolate columns */
  for (int x = 0; x < TP_HEIGHT; x += n) {
    for (int y = 0; y < TP_WIDTH; y += n) {
      /* point values */
      float p0 = tpm_get_pixel(dst, x, y - n);
      float p1 = tpm_get_pixel(dst, x, y);
      float p2 = tpm_get_pixel(dst, x, y + n);
      float p3 = tpm_get_pixel(dst, x, y + n * 2);

      /* Catmull-Rom tangents */
      float t1 = 0.5f * (p0 + p1);
      float t2 = 0.5f * (p1 + p3);

      for (int i = 0; i < n; i++)
        tpm_put_pixel(dst, x, y + i,
                      tpm_bezier_interpolate(i * ds, p1, p2, t1, t2));
    }
  }

  /* interpolate rows */
  for (int y = 0; y < TP_WIDTH; y++) {
    for (int x = 0; x < TP_HEIGHT; x += n) {
      /* point values */
      float p0 = tpm_get_pixel(dst, x - n, y);
      float p1 = tpm_get_pixel(dst, x, y);
      float p2 = tpm_get_pixel(dst, x + n, y);
      float p3 = tpm_get_pixel(dst, x + 2 * n, y);

      /* Catmull-Rom tangents */
      float t1 = 0.5f * (p0 + p1);
      float t2 = 0.5f * (p1 + p3);

      for (int i = 0; i < n; i++)
        tpm_put_pixel(dst, x + i, y,
                      tpm_bezier_interpolate(i * ds, p1, p2, t1, t2));
    }
  }
}

void tpm_perlin_noise(tpm_mono_buf dst, unsigned seed) {
  tpm_put_pixel(dst, 0, 0, tpm_random(&seed) & 255);
  tpm_put_pixel(dst, 128, 0, tpm_random(&seed) & 255);
  tpm_put_pixel(dst, 0, 128, tpm_random(&seed) & 255);
  tpm_put_pixel(dst, 128, 128, tpm_random(&seed) & 255);

  for (unsigned n = 128; n > 1; n >>= 1) {
    unsigned m = n >> 1;

    for (unsigned y = 0; y < 256; y += n) {
      for (unsigned x = 0; x < 256; x += n) {
        int p1 = tpm_get_pixel(dst, x, y);
        int p2 = tpm_get_pixel(dst, x + n, y);
        int p3 = tpm_get_pixel(dst, x, y + n);
        int p4 = tpm_get_pixel(dst, x + n, y + n);
        int r1 = (tpm_random(&seed) & (n - 1)) - m;
        int r2 = (tpm_random(&seed) & (n - 1)) - m;
        int r3 = (tpm_random(&seed) & (n - 1)) - m;
        tpm_put_pixel(dst, x + m, y, ((p1 + p2) >> 1) + r1);
        tpm_put_pixel(dst, x, y + m, ((p1 + p3) >> 1) + r2);
        tpm_put_pixel(dst, x + m, y + m, ((p1 + p2 + p3 + p4) >> 2) + r3);
      }
    }
  }
}

#if 0
void generate_cells(layer_t dst, uint8_t intens, uint8_t amount, uint16_t seed, uint16_t rseed)
{
  layer_t tmp;
  int16_t v,r;
  int32_t x,y,i,a,as,ai,xs,ys;
  float s,fr;

  Clear(dst);
  tmp = layer[12];

  s=sqrt(amount)*(2-(float)(intens)/256);

  for (y=0,i=0;y<256;y++)
    for (x=0;x<256;x++,i++)
    {
      fr=sqrt((float)((x-128)*(x-128)+(y-128)*(y-128)))*s;
      v=(int16_t)(259.75*exp(-fr*fr/16/4096)-4.75);
      if (v<0) v=0;
      tmp[i]=v;
    }

  as=0x10000/amount;

  for (a=0,ai=seed;a<amount*2;a+=2,ai+=as+seed)
  {
    Random(&seed,rseed);
    xs=(ai&0xff)-127+((seed&31)-16);
    Random(&seed,rseed);
    ys=((ai>>8)&0xff)-127+((seed&31)-16);
    for (y=0,i=0;y<256;y++) {
      for (x=0;x<256;x++,i++) {
        v=dst[i];
        r=GetPix(tmp,x+xs,y+ys);
        if (v>r) v-=r;
        if (v<r) v=r;
        if (v>255) v=255;
        dst[i]=v;
      }
    }
  }

  for (i=0;i<0x10000;i++) dst[i]=~dst[i];
}
#endif
