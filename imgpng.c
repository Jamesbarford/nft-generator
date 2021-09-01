#include <errno.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imgpng.h"
#include "panic.h"

png_byte **pngAllocRows(png_struct *png_ptr, png_info *info_ptr, int height) {
    png_byte **rows;
    if ((rows = (png_byte **)malloc(sizeof(png_byte *) * height)) == NULL)
        return NULL;

    for (int y = 0; y < height; y++)
        rows[y] = (png_byte *)malloc(png_get_rowbytes(png_ptr, info_ptr));

    return rows;
}

void printPixel(int x, int y, png_byte *pixel) {
    printf("[%d, %d] rgba(%d, %d, %d, %d)\n", x, y, pixel[R], pixel[G],
           pixel[B], pixel[A]);
}
int imgpngAllocRows(imgpng *img) {
    if ((img->rows = pngAllocRows(img->png_ptr, img->info, img->height)) ==
        NULL)
        return -1;
    return 1;
}

imgpng *imgpngCreate() {
    imgpng *img;

    if ((img = malloc(sizeof(imgpng))) == NULL) {
        return NULL;
    }

    img->height = 0;
    img->width = 0;
    return img;
}

void imgpngRowsRelease(int height, png_byte **rows) {
    for (int i = 0; i < height; i++)
        free(rows[i]);
    free(rows);
}

void imgpngRelease(imgpng *img) {
    if (img) {
        imgpngRowsRelease(img->height, img->rows);
        png_free(img->png_ptr, NULL);
        free(img);
    }
}

imgpngBasic *imgpngBasicCreate(int width, int height) {
    imgpngBasic *imgb;

    if ((imgb = (imgpngBasic *)malloc(sizeof(imgpngBasic))) == NULL)
        return NULL;

    imgb->width = width;
    imgb->height = height;
    return imgb;
}

void imgpngBasicRelease(imgpngBasic *imgb) {
    if (imgb) {
        imgpngRowsRelease(imgb->height, imgb->rows);
        free(imgb);
    }
}

imgpng *imgpngCreateFromFile(char *file_name) {
    unsigned char header[8]; // 8 is the maximum size that can be checked
    imgpng *img;

    if ((img = imgpngCreate()) == NULL)
        panic("Failed to create imgpng: %s\n", strerror(errno));

    /* open file and test for it being a png */
    FILE *fp = fopen(file_name, "rb");
    if (!fp)
        panic("Read Error: File %s could not be opened for reading", file_name);

    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8))
        panic("Read Error: File %s is not recognized as a PNG file", file_name);

    /* initialize stuff */
    img->png_ptr =
        png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!img->png_ptr)
        panic("Read Error: png_create_read_struct failed");

    img->info = png_create_info_struct(img->png_ptr);
    if (!img->info)
        panic("Read Error: png_create_info_struct failed");

    if (setjmp(png_jmpbuf(img->png_ptr)))
        panic("Read Error: Error during init_io");

    png_init_io(img->png_ptr, fp);
    png_set_sig_bytes(img->png_ptr, 8);
    png_read_info(img->png_ptr, img->info);

    img->width = png_get_image_width(img->png_ptr, img->info);
    img->height = png_get_image_height(img->png_ptr, img->info);
    img->colortype = png_get_color_type(img->png_ptr, img->info);
    img->bitdepth = png_get_bit_depth(img->png_ptr, img->info);
    img->numpasses = png_set_interlace_handling(img->png_ptr);

    png_read_update_info(img->png_ptr, img->info);

    /* read file */
    if (setjmp(png_jmpbuf(img->png_ptr)))
        panic("Read Error: Error during read_image");

    if (imgpngAllocRows(img) == -1)
        panic("Failed to allocate rows\n");
    png_read_image(img->png_ptr, img->rows);

    fclose(fp);
    return img;
}

void imgWriteToFile(int width, int height, png_byte **rows, png_byte bitdepth,
                    png_byte colortype, char *file_name) {
    FILE *fp = fopen(file_name, "wb");
    png_structp png_ptr;
    png_infop info_ptr;

    if (!fp)
        panic("Write Error: File %s could not be opened for writing",
              file_name);

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        panic("Write Error: png_create_write_struct failed");

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        panic("Write Error: png_create_info_struct failed");

    if (setjmp(png_jmpbuf(png_ptr)))
        panic("Write Error: during init_io");

    /* write header */
    if (setjmp(png_jmpbuf(png_ptr)))
        panic("Write Error: during writing header");

    png_set_IHDR(png_ptr, info_ptr, width, height, bitdepth, colortype,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);

    png_set_rows(png_ptr, info_ptr, rows);
    png_init_io(png_ptr, fp);
    png_write_info(png_ptr, info_ptr);

    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr)))
        panic("Write Error: during writing bytes");

    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    /* end write */
    if (setjmp(png_jmpbuf(png_ptr)))
        panic("Write Error: during end of write");

    png_write_end(png_ptr, NULL);

    fclose(fp);
}

void colourCheck(imgpng *img) {
    if (png_get_color_type(img->png_ptr, img->info) == PNG_COLOR_TYPE_RGB)
        panic("Processing Error: input file is PNG_COLOR_TYPE_RGB but must be "
              "PNG_COLOR_TYPE_RGBA "
              "(lacks the alpha channel)");

    if (png_get_color_type(img->png_ptr, img->info) != PNG_COLOR_TYPE_RGBA)
        panic("Processing Error: color_type of input file must be "
              "PNG_COLOR_TYPE_RGBA (%d) (is %d)",
              PNG_COLOR_TYPE_RGBA, png_get_color_type(img->png_ptr, img->info));
}
