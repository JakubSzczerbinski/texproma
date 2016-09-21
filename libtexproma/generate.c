#include "libtexproma_private.h"

void tpm_plasma(tpm_mono_buf dst, unsigned xsines, unsigned ysines) {
  xsines = constrain(xsines, 1, 100);
  ysines = constrain(ysines, 1, 100);

  float dw = 1.0f / (float)TP_WIDTH;
  float dh = 1.0f / (float)TP_HEIGHT;

  for (int y = 0; y < TP_WIDTH; y++) {
    float yrad = (float)ysines * ((float)y * dh) * M_PI; 

    for (int x = 0; x < TP_WIDTH; x++) {
      float xrad = (float)xsines * ((float)x * dw) * M_PI;

      int pixel = 0.5f * (sinf(xrad) + sinf(yrad)) * 127.0f + 127.0f;
      tpm_put_pixel(dst, x, y, pixel);
    }
  }
}

void tpm_light(tpm_mono_buf dst, unsigned type, float radius) {
  type = constrain(type, 0, 1);
  radius = constrain(radius, 0.01f, 2.0f);

  float s = 3.0f - radius;

  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x++) {
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

void tpm_perlin_plasma(tpm_mono_buf dst, unsigned step, unsigned seed) {
  for (int y = 0; y < TP_HEIGHT; y += step)
    for (int x = 0; x < TP_WIDTH; x += step)
      tpm_put_pixel(dst, x, y, tpm_random(&seed));

  for (int x = 0; x < TP_HEIGHT; x += step) {
    for (int y = 0; y < TP_WIDTH; y += step) {
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

  for (int y = 0; y < TP_HEIGHT; y++) {
    for (int x = 0; x < TP_WIDTH; x += step) {
      float p1 = tpm_get_pixel(dst, x, y);
      float p2 = tpm_get_pixel(dst, x + step, y);

      float t1 = 0.5f * (p2 - (float)tpm_get_pixel(dst, x - step, y));
      float t2 = 0.5f * ((float)tpm_get_pixel(dst, x + (step << 1), y) - p1);

      float ds = 1.0f / (float)step;

      float s; int i;

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
