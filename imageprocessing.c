#include <png.h>
#include <stdlib.h>
#include <math.h>

#include "list.h"
#include "palettes.h"
#include "imageprocessing.h"
#include "imgpng.h"

#define assignRGB(x, y) ((x)[R] = (y)[R], (x)[G] = (y)[G], (x)[B] = (y)[B])

void imgEditFile(imgpng *img) {
	colourCheck(img);

	png_byte *row;
	png_byte *pixel;
	for (int y = 0; y < img->height; y++) {
		row = img->rows[y];

		for (int x = 0; x < img->width; x++) {
			pixel = &(row[x * 4]);
			pixel[R] = 120;
			pixel[G] = pixel[B];
		}
	}
}

void imgpngBasicInit(imgpng *img, imgpngBasic *imgbasic, int scale) {
	colourCheck(img);
	imgbasic->height = scale != -1 ? img->width / scale : img->width;
	imgbasic->width = scale != -1 ? img->height / scale : img->height;

	png_struct *png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL,
			NULL);
	imgbasic->rows = pngAllocRows(png_ptr, img->info, imgbasic->height);
}

/* resize a png */
imgpngBasic *imgScaleImage(imgpng *img, int scale) {
	imgpngBasic *imgbasic = imgpngBasicCreate(img->width / scale,
			img->height / scale);

	png_struct *png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	imgbasic->rows = pngAllocRows(png_ptr, img->info, imgbasic->height);

	png_byte *row;
	png_byte *origrow;
	png_byte *pixel;
	png_byte *origpixel;

	for (int y = 0; y < imgbasic->height; ++y) {
		row = imgbasic->rows[y];
		origrow = img->rows[y * scale];

		for (int x = 0; x < imgbasic->width; ++x) {
			pixel = &(row[x * 4]);
			origpixel = &(origrow[x * 4 * scale]);
			assignRGB(pixel, origpixel);
			pixel[A] = origpixel[A];
		}
	}

	return imgbasic;
}

/**
 * This is quite a simple algorithm and the results are a bit choppy
 */
void pixilateImage(int width, int height, png_byte **rows, int scale) {
	png_byte *row;
	png_byte *origrow;
	png_byte *pixel;
	png_byte *origpixel;

	for (int y = 0; y < height; y += scale) {
		origrow = rows[y];
		for (int x = 0; x < width; x += scale) {
			origpixel = &(origrow[x * 4]);

			for (int y2 = y; (y2 < y + scale) && y2 < height; ++y2) {
				row = rows[y2];
				for (int x2 = x; (x2 < x + scale) && x2 < width; ++x2) {
					pixel = &(row[x2 * 4]);
					assignRGB(pixel, origpixel);
					pixel[A] = origpixel[A];
				}
			}
		}
	}
}


static inline int computeSubRGBValues(int x, int y, int width, int height,
		png_byte **rows, int scale)
{
	png_byte *origpixel;
	png_byte *row;

	int startx = x;
	int starty = y;
	int subimageWidth = scale;
	int subimageHeight = scale;
	int sumR = 0;
	int sumG = 0;
	int sumB = 0;
	int sum = 0;

	if (startx > width) startx = width;
	if (starty > height) starty = height;
	if ((startx + subimageWidth) > width) {
		subimageWidth = width - startx;
	}
	if ((starty + subimageHeight) > height) {
		subimageHeight = height - starty;
	}

	for (int y2 = y; (y2 < y + scale) && y2 < height; ++y2) {
		row = rows[y2];
		for (int x2 = x; (x2 < x + scale) && x2 < width; ++x2) {
			origpixel = &(row[x2 * 4]);
			sumR += origpixel[R];
			sumG += origpixel[G];
			sumB += origpixel[B];
			sum++;
		}
	}

	return (sumR / sum) << 16 | (sumG / sum) << 8 | sumB / sum;
}

/**
 * NEW ALGO
 *
 * https://stackoverflow.com/questions/15777821/how-can-i-pixelate-a-jpg-with-java */
void pixilateImage2(int width, int height, png_byte **rows, int scale) {
	png_byte *origrow;
	png_byte *row;
	png_byte *pixel;
	png_byte *origpixel;
	int rgbSub = 0;

	for (int y = 0; y < height; y += scale) {
		origrow = rows[y];
		for (int x = 0; x < width; x += scale) {
			origpixel = &(origrow[x * 4]);
			
			rgbSub = computeSubRGBValues(x, y, width, height, rows, scale);

			for (int y2 = y; (y2 < y + scale) && y2 < height; ++y2) {
				row = rows[y2];
				for (int x2 = x; (x2 < x + scale) && x2 < width; ++x2) {
					pixel = &(row[x2 * 4]);
					pixel[R] = (rgbSub >> 16) & 0xFF;
					pixel[G] = (rgbSub >> 8) & 0xFF;
					pixel[B] = rgbSub & 0xFF;
					pixel[A] = origpixel[A];
				}
			}
		}
	}
}


static int getSimilarColor(int *rgbColors, int rgbColorSize,
		int *comparitorColor)
{
	int similar = 0;
	for (int i = 0; i < rgbColorSize; ++i) {
		similar += (rgbColors[i] - comparitorColor[i]) * 
			(rgbColors[i] - comparitorColor[i]);
	}

	return sqrt(similar);
}

/**
 * Find the colours in the pallete most suited to the actual colour.
 *
 * **palette is an array of int arrays
 * **out is a pointer to an int array
 */
static void getSelectedColor(int *actualColors, int actualColorsSize,
		colorPalette *palette, int **out)
{
	int cur = getSimilarColor(actualColors, actualColorsSize, palette->colors[0]);
	int next = 0;
	for (int i = 0; i < palette->size; ++i) {

		if ((next = getSimilarColor(actualColors, actualColorsSize,
						palette->colors[i])) <= cur)
		{
			*out = palette->colors[i];
			cur = next;
		}
	}
}

void coloriseImage(int width, int height, png_byte **rows,
		colorPalette *palette)
{
	png_byte *row;
	png_byte *pixel;
	// just to silence gcc
	int *out = {0};

	for (int y = 0; y < height; ++y) {
		row = rows[y];

		for (int x = 0; x < width; ++x) {
			pixel = &(row[x * 4]);

			getSelectedColor((int *)pixel, 3, palette, &out);
			assignRGB(pixel, out);
			pixel[A] = pixel[A];
		}
	}
}

/* this is much much closer*/
void coloriseImage2(int width, int height, png_byte **rows,
		colorPalette *palette, int scale)
{
	png_byte *origrow;
	png_byte *row;
	png_byte *pixel;
	png_byte *origpixel;
	int rgbSub = 0;
	int *out;
	int rgbarr[3];

	for (int y = 0; y < height; y+=scale) {
		origrow = rows[y];
		for (int x = 0; x < width; x+=scale) {
			origpixel = &(origrow[x * 4]);
			
			rgbSub = computeSubRGBValues(x, y, width, height, rows, scale);

			rgbarr[R] = (rgbSub >> 16) & 0xFF;
			rgbarr[G] = (rgbSub >> 8) & 0xFF;
			rgbarr[B] = rgbSub & 0xFF;

			getSelectedColor(rgbarr, 3, palette, &out);

			for (int y2 = y; (y2 < y + scale) && y2 < height; ++y2) {
				row = rows[y2];
				for (int x2 = x; (x2 < x + scale) && x2 < width; ++x2) {
					pixel = &(row[x2 * 4]);
					assignRGB(pixel, out);
					pixel[A] = origpixel[A];
				}
			}
		}
	}
}

/* this is much faster than the above and looks nicer */
void coloriseImage3(int width, int height, png_byte **rows,
		colorPalette *palette, int scale)
{
	png_byte *row;
	png_byte *origrow;
	png_byte *pixel;
	png_byte *origpixel;
	int rgbarr[3];
	int *out;

	for (int y = 0; y < height; y += scale) {
		origrow = rows[y];
		for (int x = 0; x < width; x += scale) {
			origpixel = &(origrow[x * 4]);

			assignRGB(rgbarr, origpixel);
			
			getSelectedColor(rgbarr, 3, palette, &out);

			for (int y2 = y; (y2 < y + scale) && y2 < height; ++y2) {
				row = rows[y2];
				for (int x2 = x; (x2 < x + scale) && x2 < width; ++x2) {
					pixel = &(row[x2 * 4]);
					assignRGB(pixel, out);
					pixel[A] = origpixel[A];
				}
			}
		}
	}
}

void greyscaleImage(int width, int height, png_byte **rows) {
	png_byte *row;
	png_byte *pixel;
	int avg;

	for (int y = 0; y < height; ++y) {
		row = rows[y];
		for (int x = 0; x < width; ++x) {
			pixel = &(row[x * 4]);
			avg = pixel[R] + pixel[G] + pixel[B];
			pixel[R] = avg;
			pixel[G] = avg;
			pixel[B] = avg;
			pixel[A] = pixel[A];
		}
	}
}
