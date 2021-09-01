#ifndef __IMAGE_PROCESSING_H__
#define __IMAGE_PROCESSING_H__

#include "palettes.h"
#include "imgpng.h"

void imgEditFile(imgpng *img);

void imgpngBasicInit(imgpng *img, imgpngBasic *imgb, int scale);
/**
 * This is quite a simple algorithm and the results are a bit choppy
 */
void pixilateImage(int width, int height, png_byte **rows, int scale);

/**
 * NEW ALGO
 *
 * https://stackoverflow.com/questions/15777821/how-can-i-pixelate-a-jpg-with-java */
void pixilateImage2(int width, int height, png_byte **rows, int scale);

/* resize a png */
imgpngBasic *imgScaleImage(imgpng *img, int scale);

void coloriseImage(int width, int height, png_byte **rows,
		colorPalette *palette);

/* this is much much closer*/
void coloriseImage2(int width, int height, png_byte **rows,
		colorPalette *palette, int scale);

/* this is much faster than the above and looks nicer */
void coloriseImage3(int width, int height, png_byte **rows,
		colorPalette *palette, int scale);

void greyscaleImage(int width, int height, png_byte **rows);


#endif
