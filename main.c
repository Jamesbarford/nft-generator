#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <png.h>
#include <errno.h>
#include <time.h>
#include <math.h>

#define PNG_DEBUG 3
#define R 0
#define G 1
#define B 2
#define A 3

void abort_(const char * s, ...) {
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

typedef struct imgpng {
	int width;
	int height;
	int numpasses;
	png_info *info;
	png_byte colortype;
	png_byte bitdepth;
	png_byte **rows;
	png_struct *png_ptr;
} imgpng;

typedef struct imgpngBasic {
	int width;
	int height;
	png_byte **rows;
} imgpngBasic;


static int examplePalette[][3] = {
		{244,  32,   12 },
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

void imgpngRowsRelease(int height, png_byte **rows) {
	for (int i = 0; i < height; i++)
		free(rows[i]);
	free(rows);
}

png_byte **pngAllocRows(png_struct *png_ptr, png_info *info_ptr, int height) {
	png_byte **rows;
	if ((rows = (png_byte **) malloc(sizeof(png_byte *) * height)) == NULL)
		return NULL;

	for (int y = 0; y < height; y++)
		rows[y] = (png_byte *) malloc(png_get_rowbytes(png_ptr, info_ptr));

	return rows;
}

int imgpngAllocRows(imgpng *img) {
	if ((img->rows = pngAllocRows(img->png_ptr, img->info, img->height)) == NULL)
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

void printPixel(int x, int y, png_byte *pixel) {
	printf("[%d, %d] rgba(%d, %d, %d, %d)\n",
			x, y, pixel[R], pixel[G], pixel[B], pixel[A]);
}

imgpng *imgpngCreateFromFile(char* file_name) {
	unsigned char header[8];    // 8 is the maximum size that can be checked
	imgpng *img;

	if ((img = imgpngCreate()) == NULL)
		abort_("Failed to create imgpng: %s\n", strerror(errno));

	/* open file and test for it being a png */
	FILE *fp = fopen(file_name, "rb");
	if (!fp)
		abort_("Read Error: File %s could not be opened for reading", file_name);

	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8))
		abort_("Read Error: File %s is not recognized as a PNG file", file_name);

	/* initialize stuff */
	img->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!img->png_ptr)
		abort_("Read Error: png_create_read_struct failed");

	img->info = png_create_info_struct(img->png_ptr);
	if (!img->info)
		abort_("Read Error: png_create_info_struct failed");

	if (setjmp(png_jmpbuf(img->png_ptr)))
		abort_("Read Error: Error during init_io");

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
			abort_("Read Error: Error during read_image");

	if (imgpngAllocRows(img) == -1)
		abort_("Failed to allocate rows\n");
	png_read_image(img->png_ptr, img->rows);

	fclose(fp);
	return img;
}


void imgWriteToFile(int width, int height, png_byte **rows, png_byte bitdepth,
		png_byte colortype, char* file_name)
{
	FILE *fp = fopen(file_name, "wb");
	png_structp png_ptr;
	png_infop info_ptr;

	if (!fp)
		abort_("Write Error: File %s could not be opened for writing", file_name);

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("Write Error: png_create_write_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("Write Error: png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("Write Error: during init_io");


	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("Write Error: during writing header");

	png_set_IHDR(png_ptr, info_ptr, width, height,
							 bitdepth, colortype, PNG_INTERLACE_NONE,
							 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_set_rows(png_ptr, info_ptr, rows);
	png_init_io(png_ptr, fp);
	png_write_info(png_ptr, info_ptr);

	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("Write Error: during writing bytes");

	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);


	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("Write Error: during end of write");

	png_write_end(png_ptr, NULL);

	fclose(fp);
}


void imgEditFile(imgpng *img) {
	if (png_get_color_type(img->png_ptr, img->info) == PNG_COLOR_TYPE_RGB)
		abort_("Processing Error: input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA "
					 "(lacks the alpha channel)");

	if (png_get_color_type(img->png_ptr, img->info) != PNG_COLOR_TYPE_RGBA)
		abort_("Processing Error: color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)",
					 PNG_COLOR_TYPE_RGBA, png_get_color_type(img->png_ptr, img->info));

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

/*https://stackoverflow.com/questions/15777821/how-can-i-pixelate-a-jpg-with-java*/
static imgpngBasic *pixilateImage(imgpng *img, int scale) {
	imgpngBasic *imgscaled = imgpngBasicCreate(img->width,
			img->height);

	png_struct *png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	imgscaled->rows = pngAllocRows(png_ptr, img->info, imgscaled->height);

	png_byte *row;
	png_byte *origrow;
	png_byte *pixel;
	png_byte *origpixel;

	int avgR;
	int avgG;
	int avgB;
	int avgA;

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

			/*
		for (int y2 = y; y2 < y + scale; ++y2) {
			for (int x2 = x; x2 < x + scale; ++x2) {
				origpixel = &(origrow[x2 * 4 * scale]);
				avgR += origpixel[R];
				avgG += origpixel[G];
				avgB += origpixel[B];
				avgA += origpixel[A];
			}
		}
			origpixel = &(origrow[x * 4]);
			avgR /= scale << 1;
			avgG /= scale << 1;
			avgB /= scale << 1;
			avgA /= scale << 1;

			for (int y2 = y; y2 < y + scale; ++y2) {
				for (int x2 = x; x2 < x + scale; ++x2) {
					pixel[R] = avgR;
					pixel[G] = avgG;
					pixel[B] = avgB;
					pixel[A] = avgA;
				}
			}
			*/
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

static void outfileName(char *outbuf, int width, int height, char *fileout) {
	char timebuf[24];
	timestamp(timebuf);

	sprintf(outbuf, "%dx%d--%s--%s", width, height, timebuf, fileout);

	printf("%s\n", outbuf);
}

void pngResizeImage(char *filename, char *fileout, int scale) {
	char outbuf[BUFSIZ] = {'\0'};
	imgpng *img = imgpngCreateFromFile(filename);
	imgpngBasic *imgscaled = pixilateImage(img, 10);
//	coloriseImage(imgscaled, examplePalette, 6);

	outfileName(outbuf, imgscaled->width, imgscaled->height, fileout);
	imgWriteToFile(imgscaled->width, imgscaled->height, imgscaled->rows,
			img->bitdepth, img->colortype, outbuf);
	imgpngRelease(img);
	imgpngBasicRelease(imgscaled);
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
	for (int i = 0; i < paletteSize; ++i) {
		if (getSimilarColor(actualColors, actualColorsSize, examplePalette[i]) <= cur) {
			*out = palette[i];
			cur = getSimilarColor(actualColors, actualColorsSize, examplePalette[i]);
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

int main(int argc, char **argv) {
	if (argc != 3)
		abort_("Usage: program_name <file_in> <file_out>");

	pngResizeImage(argv[1], argv[2], 2);
	return 0;
}
