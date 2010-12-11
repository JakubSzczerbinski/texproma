#include "libtxtproc_private.h"

void Blur(uint8_t *pic,uint8_t amount)
{
  uint8_t bl,*dlr,*dlg,*dlb,*slr,*slg,*slb;
  uint16_t x,y,r,g,b;
  uint32_t  i;

  slr = layer[12];
  slg = layer[13];
  slb = layer[14];

  dlr = pic;
  dlg = pic + 0x10000;
  dlb = pic + 0x20000;

  for (bl = 0; bl < amount; bl++)
  {
    LayersCopy(slr,dlr);
    LayersCopy(slg,dlg);
    LayersCopy(slb,dlb);

    for (y = 0, i = 0; y < 256; y++)
    {
      for (x = 0; x < 256; x++, i++)
      {
        r=GetPix(slr,x-1,y-1);
        g=GetPix(slg,x-1,y-1);
        b=GetPix(slb,x-1,y-1);
        r+=GetPix(slr,x+1,y-1);
        g+=GetPix(slg,x+1,y-1);
        b+=GetPix(slb,x+1,y-1);
        r+=GetPix(slr,x,y-1);
        g+=GetPix(slg,x,y-1);
        b+=GetPix(slb,x,y-1);
        r+=GetPix(slr,x-1,y);
        g+=GetPix(slg,x-1,y);
        b+=GetPix(slb,x-1,y);
        r+=GetPix(slr,x+1,y);
        g+=GetPix(slg,x+1,y);
        b+=GetPix(slb,x+1,y);
        r+=GetPix(slr,x-1,y+1);
        g+=GetPix(slg,x-1,y+1);
        b+=GetPix(slb,x-1,y+1);
        r+=GetPix(slr,x+1,y+1);
        g+=GetPix(slg,x+1,y+1);
        b+=GetPix(slb,x+1,y+1);
        r+=GetPix(slr,x,y+1);
        g+=GetPix(slg,x,y+1);
        b+=GetPix(slb,x,y+1);

        dlr[i]=(r>>3);
        dlg[i]=(g>>3);
        dlb[i]=(b>>3);
      }
    }
  }
}

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

void Emboss(uint8_t *pic)
{
  uint8_t *dlr,*dlg,*dlb,*slr,*slg,*slb;
  uint16_t x,y;
  uint16_t pr,pg,pb,lr,lg,lb;
  int16_t  r,g,b;
  uint32_t i;

  slr=pic;
  slg=pic+0x10000;
  slb=pic+0x20000;

  dlr=layer[12];
  dlg=layer[13];
  dlb=layer[14];

  for (y=0,i=0;y<256;y++)
    for (x=0;x<256;x++,i++)
    {
      lr=GetPix(slr,x-1,y-1)+GetPix(slr,x-1,y)+GetPix(slr,x-1,y+1);
      lg=GetPix(slg,x-1,y-1)+GetPix(slg,x-1,y)+GetPix(slg,x-1,y+1);
      lb=GetPix(slb,x-1,y-1)+GetPix(slb,x-1,y)+GetPix(slb,x-1,y+1);
      pr=GetPix(slr,x+1,y-1)+GetPix(slr,x+1,y)+GetPix(slr,x+1,y+1);
      pg=GetPix(slg,x+1,y-1)+GetPix(slg,x+1,y)+GetPix(slg,x+1,y+1);
      pb=GetPix(slb,x+1,y-1)+GetPix(slb,x+1,y)+GetPix(slb,x+1,y+1);

      r=(((pr-lr)*256/3)>>8)+127;
      g=(((pg-lg)*256/3)>>8)+127;
      b=(((pb-lb)*256/3)>>8)+127;

      if (r>255) r=255;
      if (g>255) g=255;
      if (b>255) b=255;
      if (r<0) r=0;
      if (g<0) g=0;
      if (b<0) b=0;

      dlr[i]=r;
      dlg[i]=g;
      dlb[i]=b;
    }

  CopyTempLayer(pic);
}
