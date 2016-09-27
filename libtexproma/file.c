#include <stdio.h>
#include <stdlib.h>
#include <png.h>

#include "libtexproma_private.h"

static void tpm_buf_save(void *src, unsigned type, char *filename) {
  FILE *fp = fopen(filename, "wb");

  assert(fp != NULL);

  png_structp png = 
    png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  assert(png != NULL);

  png_infop info = png_create_info_struct(png);

  assert(info != NULL);
  assert(setjmp(png_jmpbuf(png)) == 0);

  png_init_io(png, fp);

  png_set_IHDR(png, info,
               TP_WIDTH, TP_HEIGHT, 8, type, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png, info);

  int row_size = TP_WIDTH *
    ((type == PNG_COLOR_TYPE_RGB) ? sizeof(png_color) : sizeof(uint8_t));

  for (int i = 0; i < TP_HEIGHT; i++)
		png_write_row(png, src + i * row_size);

  png_write_end(png, NULL);
  png_free_data(png, info, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&png, NULL);

  fclose(fp);
}

void tpm_mono_buf_save(tpm_mono_buf src, char *filename) {
  tpm_buf_save((void *)src, PNG_COLOR_TYPE_GRAY, filename);
}

void tpm_color_buf_save(tpm_color_buf src, char *filename) {
  png_color *tmp = malloc(TP_WIDTH * TP_HEIGHT * sizeof(png_color));

  for (int i = 0; i < TP_WIDTH * TP_HEIGHT; i++)
    tmp[i] = (png_color){src[0][i], src[1][i], src[2][i]};

  tpm_buf_save(tmp, PNG_COLOR_TYPE_RGB, filename);

  free(tmp);
}
