#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <png.h>
#include <errno.h>
#include <time.h>
#include <math.h>

#include "ihmap.h"
#include "imgpng.h"
#include "panic.h"

#define PNG_DEBUG 3
#define R 0
#define G 1
#define B 2
#define A 3

static int examplePalette[][3] = {
	{44, 33, 55},
  {118, 68, 98},
	{237, 180, 161},
	{169, 104, 104},
};
static void coloriseImage(imgpngBasic *imgb, int palette[][3], int paletteSize);

static int hexTable(char c) {
	switch (c) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': return 10; case 'A': return 10;
		case 'b': return 11; case 'B': return 11;
		case 'c': return 12; case 'C': return 12;
		case 'd': return 13; case 'D': return 13;
		case 'e': return 14; case 'E': return 14;
		case 'f': return 15; case 'F': return 15;
		default:
			printf("'%d' is not a valid hex value\n", c);
			exit(EXIT_FAILURE);
	}

	return 0;
}

int hexToRGBArray(char *hex, int *rgbArray) {
  if (strlen(hex) != 7 && hex[0] != '#') {
		return -1;
  }

	rgbArray[R] = (hexTable(hex[1]) << 4) | hexTable(hex[2]);
	rgbArray[G] = (hexTable(hex[3]) << 4) | hexTable(hex[4]);
	rgbArray[B] = (hexTable(hex[5]) << 4) | hexTable(hex[6]);
	return 1;
}

static void timestamp(char *timebuf) {
	time_t raw = time(NULL);	

	if (raw == -1) {
		exit(EXIT_FAILURE);
	}

	struct tm *ptm = localtime(&raw);
	if (ptm == NULL) {
		exit(EXIT_FAILURE);
	}

	snprintf(timebuf, 16, "%d-%02d-%dT%02d:%02d:%02d",
			ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour,
			ptm->tm_min, ptm->tm_sec);
}

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

/**
 * This is quite a simple algorithm and the results are a bit choppy
 */
static imgpngBasic *pixilateImage(imgpng *img, int scale) {
	imgpngBasic *imgscaled = imgpngBasicCreate(img->width,
			img->height);

	png_struct *png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL,
			NULL);
	imgscaled->rows = pngAllocRows(png_ptr, img->info, imgscaled->height);

	png_byte *row;
	png_byte *origrow;
	png_byte *pixel;
	png_byte *origpixel;

	for (int y = 0; y < imgscaled->height; y+=scale) {
		origrow = img->rows[y];
		for (int x = 0; x < imgscaled->width; x+=scale) {
			origpixel = &(origrow[x * 4]);

			for (int y2 = y; (y2 < y + scale) && y2 < imgscaled->height; ++y2) {
				row = imgscaled->rows[y2];
				for (int x2 = x; (x2 < x + scale) && x2 < imgscaled->width; ++x2) {
					pixel = &(row[x2 * 4]);
					pixel[R] = origpixel[R];
					pixel[G] = origpixel[G];
					pixel[B] = origpixel[B];
					pixel[A] = origpixel[A];
				}
			}
		}
	}

	return imgscaled;
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
static imgpngBasic *pixilateImage2(imgpng *img, int scale) {
	imgpngBasic *imgscaled = imgpngBasicCreate(img->width,
			img->height);

	png_struct *png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL,
			NULL);
	imgscaled->rows = pngAllocRows(png_ptr, img->info, imgscaled->height);

	png_byte *origrow;
	png_byte *row;
	png_byte *pixel;
	png_byte *origpixel;
	int rgbSub = 0;

	for (int y = 0; y < imgscaled->height; y+=scale) {
		origrow = img->rows[y];
		for (int x = 0; x < imgscaled->width; x+=scale) {
			origpixel = &(origrow[x * 4]);
			
			rgbSub = computeSubRGBValues(x, y, imgscaled->width, imgscaled->height,
					img->rows, scale);

			for (int y2 = y; (y2 < y + scale) && y2 < imgscaled->height; ++y2) {
				row = imgscaled->rows[y2];
				for (int x2 = x; (x2 < x + scale) && x2 < imgscaled->width; ++x2) {
					pixel = &(row[x2 * 4]);
					pixel[R] = (rgbSub >> 16) & 0xFF;
					pixel[G] = (rgbSub >> 8) & 0xFF;
					pixel[B] = rgbSub & 0xFF;
					pixel[A] = origpixel[A];
				}
			}
		}
	}

	return imgscaled;
}

/* resize a png */
imgpngBasic *imgScaleImage(imgpng *img, int scale) {
	imgpngBasic *imgscaled = imgpngBasicCreate(img->width / scale,
			img->height / scale);

	png_struct *png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	imgscaled->rows = pngAllocRows(png_ptr, img->info, imgscaled->height);

	png_byte *row;
	png_byte *origrow;
	png_byte *pixel;
	png_byte *origpixel;

	for (int y = 0; y < imgscaled->height; ++y) {
		row = imgscaled->rows[y];
		origrow = img->rows[y * scale];

		for (int x = 0; x < imgscaled->width; ++x) {
			pixel = &(row[x * 4]);
			origpixel = &(origrow[x * 4 * scale]);

			pixel[R] = origpixel[R];
			pixel[G] = origpixel[G];
			pixel[B] = origpixel[B];
			pixel[A] = origpixel[A];
		}
	}

	return imgscaled;
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
		int palette[][3], int paletteSize, int **out)
{
	int cur = getSimilarColor(actualColors, actualColorsSize, examplePalette[0]);
	int next = 0;
	for (int i = 0; i < paletteSize; ++i) {

		if ((next = getSimilarColor(actualColors, actualColorsSize,
						examplePalette[i])) <= cur)
		{
			*out = palette[i];
			cur = next;
		}
	}
}

static void coloriseImage(imgpngBasic *imgb, int palette[][3], int paletteSize) {
	png_byte *row;
	png_byte *pixel;
	int *out;

	for (int y = 0; y < imgb->height; ++y) {
		row = imgb->rows[y];

		for (int x = 0; x < imgb->width; ++x) {
			pixel = &(row[x * 4]);

			getSelectedColor((int *)pixel, 3, palette, paletteSize, &out);

			pixel[R] = out[R];
			pixel[G] = out[G];
			pixel[B] = out[B];
			pixel[A] = pixel[A];
		}
	}
}

/* this is much much closer*/
static imgpngBasic *coloriseImage2(imgpng *img, int palette[][3], int paletteSize) {
	imgpngBasic *imgscaled = imgpngBasicCreate(img->width,
			img->height);

	png_struct *png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL,
			NULL);
	imgscaled->rows = pngAllocRows(png_ptr, img->info, imgscaled->height);

	png_byte *origrow;
	png_byte *row;
	png_byte *pixel;
	png_byte *origpixel;
	int rgbSub = 0;
	int *out;
	int rgbarr[3];

	for (int y = 0; y < imgscaled->height; y+=10) {
		origrow = img->rows[y];
		for (int x = 0; x < imgscaled->width; x+=10) {
			origpixel = &(origrow[x * 4]);
			
			rgbSub = computeSubRGBValues(x, y, imgscaled->width, imgscaled->height,
					img->rows, 20);

			rgbarr[R] = (rgbSub >> 16) & 0xFF;
			rgbarr[G] = (rgbSub >> 8) & 0xFF;
			rgbarr[B] = rgbSub & 0xFF;

			getSelectedColor(rgbarr, 3, palette, paletteSize, &out);

			for (int y2 = y; (y2 < y + 20) && y2 < imgscaled->height; ++y2) {
				row = imgscaled->rows[y2];
				for (int x2 = x; (x2 < x + 20) && x2 < imgscaled->width; ++x2) {
					pixel = &(row[x2 * 4]);
					pixel[R] = out[R];
					pixel[G] = out[G];
					pixel[B] = out[B];
					pixel[A] = origpixel[A];
				}
			}
		}
	}

	return imgscaled;
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

static void outfileName(char *outbuf, int width, int height, char *fileout) {
	char timebuf[24];
	timestamp(timebuf);

	sprintf(outbuf, "%dx%d--%s--%s", width, height, timebuf, fileout);

	printf("%s\n", outbuf);
}

void imgpngColorise(imgpng *img, int pallete[][3], int paletteSize) {
	imgpngBasic imgb;
	imgb.height = img->height;
	imgb.width = img->width;
	imgb.rows = img->rows;
	coloriseImage(&imgb, pallete, paletteSize);
}

void process(char *filename, char *fileout) {
	char outbuf[BUFSIZ] = {'\0'};
	int width;
	int height;
	png_byte **rows;

	imgpng *img = imgpngCreateFromFile(filename);
	

	imgScaleImage(img, 8);
	

	// greyscaleImage(img->width, img->height, img->rows);
	// imgpngColorise(img, examplePalette, 4);
	// imgEditFile(img);

	width = img->width;
	height = img->height;
	rows = img->rows;

	imgpngBasic *imgb = coloriseImage2(img, examplePalette, 4);
	//imgb = pixilateImage2(img, 10);

	width = imgb->width;
	height = imgb->height;
	rows = imgb->rows;


//	coloriseImage(imgscaled, examplePalette, 4);

	outfileName(outbuf, width, height, fileout);
	imgWriteToFile(width, height, rows,
			img->bitdepth, img->colortype, outbuf);
	imgpngRelease(img);
	imgpngBasicRelease(imgb);
}

int main(int argc, char **argv) {
	if (argc != 3)
		panic("Usage: program_name <file_in> <file_out>");

	process(argv[1], argv[2]);
	return 0;
}
