#include "libtexproma_private.h"

#if 0
void tpm_distort_sine(uint8_t *pic, uint8_t xsines, uint8_t ysines, uint8_t
                        xamp, uint8_t yamp, uint8_t xshift, uint8_t yshift) {
  uint8_t *dlr,*dlg,*dlb,*slr,*slg,*slb;
  uint16_t x,y;
  uint32_t  i;
  float tx,ty;

  for (i=0;i<256;i++)
  {
    sinfx[i]=0.25*sin(((i+xshift)*xsines)*2*M_PI/256)*xamp;
    sinfy[i]=0.25*sin(((i+yshift)*ysines)*2*M_PI/256)*yamp;
  }

  slr=pic;
  slg=pic+0x10000;
  slb=pic+0x20000;

  dlr=layer[12];
  dlg=layer[13];
  dlb=layer[14];

  for (y=0,i=0;y<256;y++)
    for (x=0;x<256;x++,i++)
    {
      tx=(float)x+sinfx[y];
      ty=(float)y+sinfy[x];
      dlr[i]=GetFilteredPix(slr,tx,ty);
      dlg[i]=GetFilteredPix(slg,tx,ty);
      dlb[i]=GetFilteredPix(slb,tx,ty);
    }

  CopyTempLayer(pic);
}
#endif

void tpm_twist(tpm_mono_buf dst, tpm_mono_buf src, float strenght) {
  float s = strenght * M_PI / 32.0f;

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

void tpm_move(tpm_mono_buf dst, tpm_mono_buf src, int xoffset, int yoffset) {
  for (int y = 0; y < TP_HEIGHT; y++)
    for (int x = 0; x < TP_WIDTH; x++)
      tpm_put_pixel(dst, x, y, tpm_get_pixel(src, (x + xoffset) % TP_WIDTH,
                                                  (y + yoffset) % TP_HEIGHT));
}

void tpm_uvmap(tpm_mono_buf dst, tpm_mono_buf src,
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
