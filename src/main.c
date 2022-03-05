#include <errno.h>
#include <png.h>
#include <pngconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "hmap.h"
#include "imageprocessing.h"
#include "imgpng.h"
#include "palettes.h"
#include "panic.h"
#include "cstr.h"

typedef struct imgProcessOpts {
    char *filename;
    char *outname;
    int scale;
    int blockSize;
    int colorflags;
    int edgedetection;
    int from;
    int to;
    int mixchannels;
    int rgbvalues;
    int merge;
    cstr **files;
    int file_count;
} imgProcessOpts;

static void usage(void) {
    printf("Usage:\n\t--file <filename> --out-file <out_filename> --block-size "
           "<int> --scale <int>\n\n"
           "Where:\n"
           "\t--file           the path to the input file\n"
           "\t--merge          comma separated list of files to merge\n\n"
           "\t--out-file       a suffix preceeding .png\n"
           "\t--block-size     optional the size of the pixel effect\n"
           "\t--scale          optional resize the image\n"
           "\t--greyscale      optional, default is color for edge detection\n"
           "\t--color          optional, default is color for edge detection\n"
           "\t--edge-detection flag to use edge detection algorithm\n"
           "\t--from           iteration to start from, applying a different "
           "blocksize at each increment\n"
           "\t--to             iteration to end\n"
           "\t--scale          optional resize the image\n"
           "\t--help           display this message"
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

    snprintf(timebuf, 72, "%d-%02d-%dT%02d:%02d:%02d", ptm->tm_year + 1900,
             ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min,
             ptm->tm_sec);
}

static void outfileName(char *outbuf, int width, int height, char *fileout,
                        int number)
{
    char timebuf[72];
    timestamp(timebuf);

    sprintf(outbuf, "%dx%d--%s--%s--%d.png", width, height, timebuf, fileout,
            number);

    printf("%s\n", outbuf);
}

void writeRowsToFile(int width, int height, char *outname, png_byte **rows,
                     imgpng *original, int fileno)
{
    char outbuf[BUFSIZ] = {'\0'};

    outfileName(outbuf, width, height, outname, fileno);
    imgWriteToFile(width, height, rows, original->bitdepth, original->colortype,
                   outbuf);
}

void generatePixlatedPngs(hmap *paletteMap, imgpng *original,
        imgProcessOpts *opts, int blocksize)
{
    hmapEntry *he;
    colorPalette *palette;
    imgpngBasic *imgb;
    char key[4] = {'\0'};
    int width;
    int height;
    png_byte **rows;

    for (unsigned int i = 0; i < paletteMap->size; ++i) {
        snprintf(key, 4, "%d", i + 1);
        he = hmapGetValue(paletteMap, key);
        palette = he->value;

        imgb = imgScaleImage(original, opts->scale);

        coloriseImage2(imgb->width, imgb->height, imgb->rows, palette,
                       blocksize);
        width = imgb->width;
        height = imgb->height;
        rows = imgb->rows;

        writeRowsToFile(width, height, opts->outname, rows, original, i);
        imgpngBasicRelease(imgb);
    }
}

/**
 * Either were generating a range of images  or just one
 * This is here as it is extremely slow to loop over this programme in bash
 */
void processPixelImages(imgProcessOpts *opts) {
    hmap *paletteMap = colorPaletteMapCreate();
    imgpng *img = imgpngCreateFromFile(opts->filename);

    if (opts->from == 0 && opts->to == 1) {
        generatePixlatedPngs(paletteMap, img, opts, opts->blockSize);
    } else {
        printf("hammertime\n");
        for (int blocksize = opts->from; blocksize < opts->to; ++blocksize) {
            generatePixlatedPngs(paletteMap, img, opts, blocksize);
        }
    }

    hmapRelease(paletteMap);
    imgpngRelease(img);
}

void edgeDetection(imgProcessOpts *opts) {
    imgpng *img = imgpngCreateFromFile(opts->filename);
    imgpng *img2 = imgpngCreateFromFile(opts->filename);
    imgpng *img3 = imgpngCreateFromFile(opts->filename);
    imgpng *img4 = imgpngCreateFromFile(opts->filename);

    greyscaleImage(img->width, img->height, img->rows);
    greyscaleImage(img->width, img->height, img2->rows);
    greyscaleImage(img->width, img->height, img3->rows);
    greyscaleImage(img->width, img->height, img4->rows);

    imgEdge *ie = imgEdgeCreate(img);
    ie->rows = img2->rows;
    ie->gx = img3->rows;
    ie->gy = img4->rows;

    sobelEdgeDetection(img->width, img->height, img->rows, ie,
                       opts->colorflags);

    minMaxNoramlisation(ie->width, ie->height, ie->rows, opts->colorflags);
    greyscaleImage(ie->width, ie->height, ie->rows);
    greyscaleImage(ie->width, ie->height, ie->gx);
    greyscaleImage(ie->width, ie->height, ie->gy);

    writeRowsToFile(img->width, img->height, opts->outname, ie->rows, img, 1);
    writeRowsToFile(img->width, img->height, opts->outname, ie->gx, img, 2);
    writeRowsToFile(img->width, img->height, opts->outname, ie->gy, img, 3);

    imgpngRelease(img);
    imgpngRelease(img2);
    imgpngRelease(img3);
    imgpngRelease(img4);
    imgEdgeRelease(ie);
}

/**
 * Diagonally colours an image using a hex value in steps.
 * Can then be combined to make a gif.
 */
void mixChannels(imgProcessOpts *opts) {
    if (opts->rgbvalues == 0) {
        panic("To mix rbg values please supply a hex value eg: --hex-value '#FFBBAA'\n");
    }
    imgpng *img = imgpngCreateFromFile(opts->filename);
    imgpngBasic *imgb = imgScaleImage(img, opts->scale);
    int dim = imgb->width + imgb->height;
    int incr = (dim / 30);
    int iter = 10;

    /*
    imgpngMixChannelsCustom(img->width, img->height, img->rows, opts->rgbvalues);
    writeRowsToFile(img->width, img->height, opts->outname, img->rows, img, iter);
    */
    for (int i = incr; i < imgb->width + imgb->height; i += incr) {
        imgpngMixChannelsUntilHeight(imgb->width, imgb->height, imgb->rows,
                opts->rgbvalues, i);
        writeRowsToFile(imgb->width, imgb->height, opts->outname, imgb->rows, img, iter);
        ++iter;
    }
    
    imgpngRelease(img);
    imgpngBasicRelease(imgb);
}

/**
 * Layers pngs on top of eachother. Ideally they will all be of the same resolution 
 */
void mergeFiles(imgProcessOpts *opts) {
    cstr **arr = opts->files;
    imgpng **imgpngArr = malloc(sizeof(imgpng *) * opts->file_count);
    int area = 0;
    int largest = 0;
    int height = 0;
    int width = 0;

    for (int i = 0; i < opts->file_count; ++i) {
        imgpngArr[i] = imgpngCreateFromFile(arr[i]);
        if (imgpngArr[i]->height * imgpngArr[i]->width > area) {
            area = imgpngArr[i]->height * imgpngArr[i]->width;
            largest = i;
        }
    }

    height = imgpngArr[largest]->height;
    width = imgpngArr[largest]->width;
    imgpngMerge(width, height, imgpngArr, opts->file_count, largest);
    writeRowsToFile(width, height, opts->outname, imgpngArr[largest]->rows,
            imgpngArr[largest], 1);

    for (int i = 0; i < opts->file_count; ++i) {
        imgpngRelease(imgpngArr[i]);
    }
    free(imgpngArr);
}

/* default behaviour is to pixilate an image with and colour it */
int main(int argc, char **argv) {
    imgProcessOpts opts;
    opts.blockSize = 12;
    opts.scale = 2;
    opts.filename = "no_file";
    opts.outname = "no_file";
    opts.colorflags = IMG_COLOR;
    opts.edgedetection = 0;
    opts.mixchannels = 0;
    opts.rgbvalues = 0;
    opts.from = 0;
    opts.to = 1;
    opts.files = NULL;
    opts.merge = 0;
    opts.file_count = 0;

    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--file") == 0) {
            opts.filename = argv[++i];
        } else if (strcmp(argv[i], "--out-file") == 0) {
            opts.outname = argv[++i];
        } else if (strcmp(argv[i], "--scale") == 0) {
            opts.scale = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--block-size") == 0) {
            opts.blockSize = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--greyscale") == 0) {
            opts.colorflags = IMG_GREYSCALE;
        } else if (strcmp(argv[i], "--color") == 0) {
            opts.colorflags = IMG_COLOR;
        } else if (strcmp(argv[i], "--edge-detection") == 0) {
            opts.edgedetection = 1;
        } else if (strcmp(argv[i], "--from") == 0) {
            opts.from = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--to") == 0) {
            opts.to = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--mix-channels") == 0) {
            opts.mixchannels = 1;
        } else if (strcmp(argv[i], "--hex-value") == 0) {
            opts.rgbvalues = hexToRGB(argv[++i]);
        } else if (strcmp(argv[i], "--merge") == 0) {
            opts.files = cstrSplit(argv[++i], ',', &opts.file_count);
            opts.merge = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            usage();
            exit(EXIT_SUCCESS);
        }
    }

    if (opts.merge == 1) {
        mergeFiles(&opts);
        cstrArrayRelease(opts.files, opts.file_count);
        return 0;
    }

    if (strcmp(opts.filename, "no_file") == 0 ||
            strcmp(opts.filename, "no_file") == 0) {
        usage();
        exit(EXIT_FAILURE);
    }

    if (opts.mixchannels == 1) {
        mixChannels(&opts);
        return 0;
    } 
    if (opts.edgedetection == 1) {
        edgeDetection(&opts);
        return 0;
    } else {
        processPixelImages(&opts);
    }
    return 0;
}