#include <stdlib.h>

#include "libtexproma_private.h"

static float _blur_3x3[] = {
  0, 1, 0,
  1, 1, 1,
  0, 1, 0,
};

static float _blur_5x5[] = {
  0, 0, 1, 0, 0,
  0, 1, 1, 1, 0,
  1, 1, 1, 1, 1,
  0, 1, 1, 1, 0,
  0, 0, 1, 0, 0,
};

static float _gaussian_3x3[] = {
  1, 2, 1,
  2, 4, 2,
  1, 2, 1,
};

static float _gaussian_5x5[] = {
  1,  4,  7,  4, 1,
  4, 16, 26, 16, 4,
  7, 26, 41, 26, 7,
  4, 16, 26, 16, 4,
  1,  4,  7,  4, 1,
};

static float _sharpen[] = {
  -1, -1, -1,
  -1,  9, -1,
  -1, -1, -1,
};

static float _emboss[] = {
  -1, -1,  0,
  -1,  0,  1,
   0,  1,  1,
};

static float _edges[] = {
  -1, -1, -1,
  -1,  8, -1,
  -1, -1, -1,
};

static float _median_3x3[] = {
  1, 1, 1,
  1, 1, 1,
  1, 1, 1,
};

static float _median_5x5[] = {
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
  1, 1, 1, 1, 1,
};

static void filter(tpm_mono_buf dst, tpm_mono_buf src,
                   float *filter, unsigned size, float factor, float bias)
{
  float *tmp = calloc(1, TP_WIDTH * TP_HEIGHT * sizeof(float));

  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++) {
      float p = tpm_get_pixel(src, x, y);

      for (unsigned j = 0; j < size; j++)
        for (unsigned i = 0; i < size; i++) {
          int k = (x + i - (size >> 1)) & (TP_WIDTH - 1);
          int l = (y + j - (size >> 1)) & (TP_HEIGHT - 1);
          tmp[k + TP_WIDTH * l] += filter[j * size + i] * p;
        }
    }

  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++)
      tpm_put_pixel(dst, x, y, tmp[x + TP_WIDTH * y] * factor + bias);

  free(tmp);
}

void tpm_blur_3x3(tpm_mono_buf dst, tpm_mono_buf src) {
  filter(dst, src, _blur_3x3, 3, 1.0f / 5.0f, 0.0f);
}

void tpm_blur_5x5(tpm_mono_buf dst, tpm_mono_buf src) {
  filter(dst, src, _blur_5x5, 5, 1.0f / 11.0f, 0.0f);
}

void tpm_gaussian_3x3(tpm_mono_buf dst, tpm_mono_buf src) {
  filter(dst, src, _gaussian_3x3, 3, 1.0f / 16.0f, 0.0f);
}

void tpm_gaussian_5x5(tpm_mono_buf dst, tpm_mono_buf src) {
  filter(dst, src, _gaussian_5x5, 5, 1.0f / 273.0f, 0.0f);
}

void tpm_sharpen(tpm_mono_buf dst, tpm_mono_buf src) {
  filter(dst, src, _sharpen, 3, 1.0f, 0.0f);
}

void tpm_emboss(tpm_mono_buf dst, tpm_mono_buf src) {
  filter(dst, src, _emboss, 3, 1.0f, 128.0f);
}

void tpm_edges(tpm_mono_buf dst, tpm_mono_buf src) {
  filter(dst, src, _edges, 3, 1.0f, 0.0f);
}

void tpm_median_3x3(tpm_mono_buf dst, tpm_mono_buf src) {
  filter(dst, src, _median_3x3, 3, 1.0f / 9.0f, 0.0f);
}

void tpm_median_5x5(tpm_mono_buf dst, tpm_mono_buf src) {
  filter(dst, src, _median_5x5, 5, 1.0f / 25.0f, 0.0f);
}

#if 0
void DirectionalBlur(uint8_t *pic,uint8_t *datalayer,uint8_t Distance)
{
  uint8_t *dlr,*dlg,*dlb,*slr,*slg,*slb;
  uint16_t v,d,x,y,r,g,b;
  uint32_t i;
  float dx,dy,px,py;

  slr=pic;
  slg=pic+0x10000;
  slb=pic+0x20000;

  dlr=layer[12];
  dlg=layer[13];
  dlb=layer[14];

  for (i=0;i<256;i++)
  {
    sinfx[i]=-sin(i*2*M_PI/256);
    sinfy[i]=-cos(i*2*M_PI/256);
  }

  for (y=0,i=0;y<256;y++)
    for (x=0;x<256;x++,i++)
    {
      v=datalayer[i];

      dx=sinfx[v];
      dy=sinfy[v];

      r=slr[i];
      g=slg[i];
      b=slb[i];

      px=dx+x;
      py=dy+y;

      for (d=1;d<=Distance;d++)
      {
        r+=GetFilteredPix(slr,px,py);
        g+=GetFilteredPix(slg,px,py);
        b+=GetFilteredPix(slb,px,py);

        px+=dx;
        py+=dy;
      }

      r/=(Distance+1);
      g/=(Distance+1);
      b/=(Distance+1);

      dlr[i]=r;
      dlg[i]=g;
      dlb[i]=b;
    }

  CopyTempLayer(pic);
}
#endif
