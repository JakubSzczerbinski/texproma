#include "libtexproma_private.h"

void tpm_plasma(tpm_mono_buf dst, uint8_t xsines, uint8_t ysines,
                float xphase, float yphase)
{
  int x, y;

  float dw = 1.0f / (float)TP_WIDTH;
  float dh = 1.0f / (float)TP_HEIGHT;

  for (y = 0; y < TP_WIDTH; y++) {
    float yrad = (float)ysines * ((float)y * dh + yphase) * M_PI; 

    for (x = 0; x < TP_WIDTH; x++) {
      float xrad = (float)xsines * ((float)x * dw + xphase) * M_PI;

      uint8_t pixel = 127.0f + (sinf(xrad) + sinf(yrad)) * 256.0f / 3.0f;
      tpm_put_pixel(dst, x, y, pixel);
    }
  }
}

void tpm_light(tpm_mono_buf dst, uint8_t type, float radius) {
  int x, y;
  float s = 3.0f - radius;

  for (y = 0; y < TP_HEIGHT; y++) {
    for (x = 0; x < TP_WIDTH; x++) {
      float r = sqrtf((float)((x - 128) * (x - 128) + (y - 128) * (y - 128))) * s;

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

void tpm_perlin_plasma(tpm_mono_buf dst, uint8_t step, uint32_t seed) {
  int x, y;

  for (y = 0; y < TP_HEIGHT; y += step)
    for (x = 0; x < TP_WIDTH; x += step)
      tpm_put_pixel(dst, x, y, tpm_random(&seed));

  for (x = 0; x < TP_HEIGHT; x += step) {
    for (y = 0; y < TP_WIDTH; y += step) {
      float p1 = tpm_get_pixel(dst, x, y);
      float p2 = tpm_get_pixel(dst, x, y + step);

      float t1 = 0.5f * (p2 - (float)tpm_get_pixel(dst, x, y - step));
      float t2 = 0.5f * ((float)tpm_get_pixel(dst, x, y + (step << 1)) - p1);

      float ds = 1.0f / (float)step;
      float s;

      int i;

      for (i = 0, s = 0.0f; i < step; i++, s += ds)
        tpm_put_pixel(dst, x, y + i,
                      (int)tpm_bezier_interpolate(s, p1, p2, t1, t2));
    }
  }

  for (y = 0; y < TP_HEIGHT; y++) {
    for (x = 0; x < TP_WIDTH; x += step) {
      float p1 = tpm_get_pixel(dst, x, y);
      float p2 = tpm_get_pixel(dst, x + step, y);

      float t1 = 0.5f * (p2 - (float)tpm_get_pixel(dst, x - step, y));
      float t2 = 0.5f * ((float)tpm_get_pixel(dst, x + (step << 1), y) - p1);

      float ds = 1.0f / (float)step;
      float s;

      int i;

      for (i = 0, s = 0.0f; i < step; i++, s += ds)
        tpm_put_pixel(dst, x + i, y, 
                      (int)tpm_bezier_interpolate(s, p1, p2, t1, t2));
    }
  }
}

#if 0
void generate_perlin_noise(layer_t dst, uint16_t seed, uint16_t rseed)
{
  int st,ss,stm,x,y,pix,p1,p2,p3,p4;

  Random(&seed,rseed);
  PutPix(dst,0,0,seed);
  Random(&seed,rseed);
  PutPix(dst,128,0,seed);
  Random(&seed,rseed);
  PutPix(dst,0,128,seed);
  Random(&seed,rseed);
  PutPix(dst,128,128,seed);

  for (st=128;st>1;st>>=1)
  {
    ss=st>>1;
    stm=st-1;

    for (y=0;y<256;y+=st)
      for (x=0;x<256;x+=st)
      {
        p1=GetPix(dst,x,y);
        p2=GetPix(dst,x+st,y);
        p3=GetPix(dst,x,y+st);
        p4=GetPix(dst,x+st,y+st);

        Random(&seed,rseed);
        pix=((p1+p2)>>1)+((seed&stm)-ss);
        if (pix>255) pix=255;
        if (pix<0) pix=0;
        PutPix(dst,x+ss,y,pix);

        Random(&seed,rseed);
        pix=((p1+p3)>>1)+((seed&stm)-ss);
        if (pix>255) pix=255;
        if (pix<0) pix=0;
        PutPix(dst,x,y+ss,pix);

        Random(&seed,rseed);
        pix=((p1+p2+p3+p4)>>2)+((seed&stm)-ss);
        if (pix>255) pix=255;
        if (pix<0) pix=0;
        PutPix(dst,x+ss,y+ss,pix);
      }
  }
}
#endif

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

#if 0
void insert_bitmap(layer_t dst, uint8_t w, uint8_t h, uint8_t x, uint8_t y, uint8_t *data)
{
  uint8_t ppix,p;
  int8_t  bpos;
  int32_t j, xi, yi;

  Clear(dst);

  for (yi=0,j=0,bpos=-1,ppix=0;yi<=h;yi++) {
    for (xi=0;xi<=w;xi++,bpos--)
    {
      if (bpos<0)
      {
        bpos=7;
        ppix=data[j++];
      }
      p = (ppix&(1<<bpos)) ? 0xff : 0x00;
      PutPix(dst,x+xi,y+yi,p);
    }
  }
}
#endif
