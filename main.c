#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <errno.h>
#include <time.h>

#include "hmap.h"
#include "imgpng.h"
#include "panic.h"
#include "imageprocessing.h"
#include "palettes.h"

typedef  struct imgProcessOpts {
	char *filename;
	char *outname;
	int scale;
	int blockSize;
} imgProcessOpts;


static void usage() {
	printf("Usage:\n\t--file <filename> --out-file <out_filename> --block-size <int> --scale <int>\n\n"
			"Where:\n"
			"\t--file        the path to the input file\n"
			"\t--out-file    a suffix preceeding .png\n"
			"\t--block-size  optional the size of the pixel effect\n"
			"\t--scale       optional resize the image\n"
			"\t--help        display this message"
			"\n");
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

	snprintf(timebuf, 72, "%d-%02d-%dT%02d:%02d:%02d",
			ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour,
			ptm->tm_min, ptm->tm_sec);
}

static void outfileName(char *outbuf, int width, int height, char *fileout,
		int number) {
	char timebuf[72];
	timestamp(timebuf);

	sprintf(outbuf, "%dx%d--%s--%s--%d.png", width, height, timebuf, fileout, number);

	printf("%s\n", outbuf);
}

void process(imgProcessOpts *opts) {
	hmap *paletteMap = colorPaletteMapCreate();
	hmapEntry *he;
	colorPalette *palette;
	char outbuf[BUFSIZ] = {'\0'};
	int width;
	int height;
	char key[4] = {'\0'};
	png_byte **rows;
	imgpng *img;
	imgpngBasic *imgb;

	img = imgpngCreateFromFile(opts->filename);

	for (unsigned int i = 0; i < paletteMap->size; ++i) {
		snprintf(key, 4, "%d", i+1);
		he = hmapGetValue(paletteMap, key);
		palette = he->value;

		imgb = imgScaleImage(img, opts->scale);

		coloriseImage2(imgb->width, imgb->height, imgb->rows, palette, opts->blockSize);
		width = imgb->width;
		height = imgb->height;
		rows = imgb->rows;

		outfileName(outbuf, width, height, opts->outname, i);
		imgWriteToFile(width, height, rows,
				img->bitdepth, img->colortype, outbuf);
		imgpngBasicRelease(imgb);
	}

	hmapRelease(paletteMap);
	imgpngRelease(img);
}

int main(int argc, char **argv) {
	imgProcessOpts opts;
	opts.blockSize = 12;
	opts.scale = 2;
	opts.filename = "no_file";

	for (int i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "--file") == 0) {
			opts.filename = argv[++i];
		} else if (strcmp(argv[i], "--out-file") == 0) {
			opts.outname = argv[++i];
		} else if (strcmp(argv[i], "--scale") == 0) {
			opts.scale = atoi(argv[++i]);
		} else if (strcmp(argv[i], "--block-size") == 0) {
			opts.blockSize = atoi(argv[++i]);
		} else if (strcmp(argv[i], "--help") == 0) {
			usage();
			exit(EXIT_SUCCESS);
		}
	}

	if (strcmp(opts.filename, "no_file") == 0) {
		usage();
		exit(EXIT_FAILURE);
	}

	process(&opts);
	return 0;
}
