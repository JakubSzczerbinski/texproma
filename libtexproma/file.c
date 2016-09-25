#include <stdio.h>
#include <stdlib.h>
#include <png.h>

#include "libtexproma_private.h"

void tpm_mono_buf_save(tpm_mono_buf src, char *filename) {
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
               TP_WIDTH, TP_HEIGHT,
               8, PNG_COLOR_TYPE_GRAY,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png, info);

  uint8_t **row_pointers = calloc(TP_HEIGHT, sizeof(uint8_t *));

  for (int i = 0; i < TP_HEIGHT; i++)
    row_pointers[i] = &src[i * TP_WIDTH];

  png_write_image(png, row_pointers);
  png_write_end(png, NULL);

  free(row_pointers);

  fclose(fp);
}
