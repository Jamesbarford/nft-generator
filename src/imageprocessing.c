/**
 * nftgen: Create nfts
 *
 * Version 1.0 March 2022
 *
 * Copyright (c) 2022, James Barford-Evans
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <math.h>
#include <png.h>
#include <pngconf.h>
#include <stdlib.h>

#include "imageprocessing.h"
#include "imgpng.h"
#include "palettes.h"

#define assignRGB(x, y) ((x)[R] = (y)[R], (x)[G] = (y)[G], (x)[B] = (y)[B])
#define getPixel(r, y, x) (&((r)[y][x * 4]))

static int sobelMX[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1},
};

static int sobelMY[3][3] = {
    {-1, -2, -1},
    {0, 0, 0},
    {1, 2, 1},
};

static inline int isNullPixel(png_byte *pixel) {
    return pixel[R] == 255 &&
           pixel[G] == 255 &&
           pixel[B] == 255 &&
           pixel[A] == 0 ? 1 : 0;
}

void imgpngMixChannels(int width, int height, png_byte **rows) {
    png_byte *pixel;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixel = getPixel(rows, y, x);
            pixel[R] = 120;
            pixel[G] = pixel[B];
        }
    }
}

/**
 * Apply a user defined colour change to each pixel
 */
void imgpngMixChannelsCustom(int width, int height, png_byte **rows, int rgb) {
    png_byte *pixel;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            pixel = getPixel(rows, y, x);
            pixel[R] = ((rgb >> 16) & 0xFF) | pixel[R];
            pixel[G] = ((rgb >> 12) & 0xFF) | pixel[G];
            pixel[B] = (rgb & 0xFF) | pixel[B];
        }
    }
}

/**
 * We go over the array diagonally, this is nice if we want to apply stripes.
 * Or create a gif that has a nice flowy effect
 */
void imgpngMixChannelsUntilHeight(int width, int height, png_byte **rows,
        int rgb, int untilHeight)
{
    png_byte *pixel;
    for (int y = 0; y <= (height + width) - 2; ++y) {
        for (int x = 0; x <= y; ++x) {
            int i = y - x;
            if (i < height && x < width) {
                pixel = getPixel(rows, i, x);
                pixel[R] = ((rgb >> 16) & 0xFF) | pixel[R];
                pixel[G] = ((rgb >> 12) & 0xFF) | pixel[G];
                pixel[B] = (rgb & 0xFF) | pixel[B]; 
            }
            if (i == untilHeight) return;
        }
    }
}

/**
 * Layer pngs on top of eachother, the largest is used as the base image.
 * If the images are all of the same resolution make sure they are passed in
 * the order you want them in for them to layer properly.
 */
void imgpngMerge(int width, int height, imgpng **imgs, int imgCount,
        int largest)
{
    (void)width;
    (void)height;
    png_byte *pixel;
    png_byte *basepxl;

    for (int i = 0; i < imgCount; ++i) {
        for (int y = 0; y < imgs[i]->height; ++y) {
            for (int x = 0; x < imgs[i]->width; ++x) {
                pixel = getPixel(imgs[i]->rows, y, x);
                basepxl = getPixel(imgs[largest]->rows, y, x);
                if (pixel[A] == 0) continue;
                basepxl[R] = pixel[R];
                basepxl[G] = pixel[G];
                basepxl[B] = pixel[B];
                basepxl[A] = pixel[A];
            }
        }

    } 
}

void imgpngBasicInit(imgpng *img, imgpngBasic *imgbasic, int scale) {
    colourCheck(img);
    imgbasic->height = scale != -1 ? img->width / scale : img->width;
    imgbasic->width = scale != -1 ? img->height / scale : img->height;

    png_struct *png_ptr =
        png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    imgbasic->rows = pngAllocRows(png_ptr, img->info, imgbasic->height);
}

/* resize a png */
imgpngBasic *imgScaleImage(imgpng *img, int scale) {
    imgpngBasic *imgbasic =
        imgpngBasicCreate(img->width / scale, img->height / scale);

    png_struct *png_ptr =
        png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    imgbasic->rows = pngAllocRows(png_ptr, img->info, imgbasic->height);

    png_byte *pixel;
    png_byte *origpixel;

    for (int y = 0; y < imgbasic->height; ++y) {
        png_byte *row = img->rows[y * scale];
        for (int x = 0; x < imgbasic->width; ++x) {
            pixel = getPixel(imgbasic->rows, y, x);
            origpixel = &(row[x * 4 * scale]);
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
    png_byte *pixel;
    png_byte *origpixel;

    for (int y = 0; y < height; y += scale) {
        for (int x = 0; x < width; x += scale) {
            origpixel = getPixel(rows, y, x);

            for (int y2 = y; (y2 < y + scale) && y2 < height; ++y2) {
                for (int x2 = x; (x2 < x + scale) && x2 < width; ++x2) {
                    pixel =getPixel(rows, y2, x2);;
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

    int startx = x;
    int starty = y;
    int subimageWidth = scale;
    int subimageHeight = scale;
    int sumR = 0;
    int sumG = 0;
    int sumB = 0;
    int sum = 0;

    if (startx > width)
        startx = width;
    if (starty > height)
        starty = height;
    if ((startx + subimageWidth) > width) {
        subimageWidth = width - startx;
    }
    if ((starty + subimageHeight) > height) {
        subimageHeight = height - starty;
    }

    for (int y2 = y; (y2 < y + scale) && y2 < height; ++y2) {
        for (int x2 = x; (x2 < x + scale) && x2 < width; ++x2) {
            origpixel = getPixel(rows, y2, x2);
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
 * https://stackoverflow.com/questions/15777821/how-can-i-pixelate-a-jpg-with-java
 */
void pixilateImage2(int width, int height, png_byte **rows, int scale) {
    png_byte *pixel;
    png_byte *origpixel;
    int rgbSub = 0;

    for (int y = 0; y < height; y += scale) {
        for (int x = 0; x < width; x += scale) {
            origpixel = getPixel(rows, y, x);
            rgbSub = computeSubRGBValues(x, y, width, height, rows, scale);

            for (int y2 = y; (y2 < y + scale) && y2 < height; ++y2) {
                for (int x2 = x; (x2 < x + scale) && x2 < width; ++x2) {
                    pixel = getPixel(rows, y2, x2);
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
    int cur =
        getSimilarColor(actualColors, actualColorsSize, palette->colors[0]);
    int next = 0;
    for (int i = 0; i < palette->size; ++i) {
        if ((next = getSimilarColor(actualColors, actualColorsSize,
                                    palette->colors[i])) <= cur) {
            *out = palette->colors[i];
            cur = next;
        }
    }
}

void coloriseImage(int width, int height, png_byte **rows,
        colorPalette *palette)
{
    png_byte *pixel;
    // just to silence gcc
    int *out = {0};

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            pixel = getPixel(rows, y, x);
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
    png_byte *pixel;
    png_byte *origpixel;
    int rgbSub = 0;
    int *out;
    int rgbarr[3];

    for (int y = 0; y < height; y += scale) {
        for (int x = 0; x < width; x += scale) {
            origpixel = getPixel(rows, y, x);

            rgbSub = computeSubRGBValues(x, y, width, height, rows, scale);

            rgbarr[R] = (rgbSub >> 16) & 0xFF;
            rgbarr[G] = (rgbSub >> 8) & 0xFF;
            rgbarr[B] = rgbSub & 0xFF;

            getSelectedColor(rgbarr, 3, palette, &out);

            for (int y2 = y; (y2 < y + scale) && y2 < height; ++y2) {
                for (int x2 = x; (x2 < x + scale) && x2 < width; ++x2) {
                    pixel = getPixel(rows, y2, x2);
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
    png_byte *pixel;
    png_byte *origpixel;
    int rgbarr[3];
    int *out;

    for (int y = 0; y < height; y += scale) {
        for (int x = 0; x < width; x += scale) {
            origpixel = getPixel(rows, y, x);

            assignRGB(rgbarr, origpixel);

            getSelectedColor(rgbarr, 3, palette, &out);

            for (int y2 = y; (y2 < y + scale) && y2 < height; ++y2) {
                for (int x2 = x; (x2 < x + scale) && x2 < width; ++x2) {
                    pixel = getPixel(rows, y2, x2);
                    assignRGB(pixel, out);
                    pixel[A] = origpixel[A];
                }
            }
        }
    }
}

static inline void setGreyscalePixel(png_byte *pixel, int color) {
    pixel[R] = color;
    pixel[G] = color;
    pixel[B] = color;
}

static inline int getGreyscalePixel(png_byte *pixel) {
    return (pixel[R] + pixel[G] + pixel[B]) / 3;
}

void greyscaleImage(int width, int height, png_byte **rows) {
    png_byte *pixel;
    int avg;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            pixel = getPixel(rows, y, x);
            avg = getGreyscalePixel(pixel);
            setGreyscalePixel(pixel, avg);
        }
    }
}

/* Apply convolution  while on the fly getting greyscale values */
static int applyConvolutionGreyscale(png_byte **rows, int kernal[3][3], int x,
        int y)
{
    int acc = 0;
    int avg;
    png_byte *pxl;

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            pxl = getPixel(rows, y + i, x + j);
            avg = getGreyscalePixel(pxl);
            acc += avg * kernal[i][j];
        }
    }

    return acc;
}

/* Apply a convolution to a given color channel */
static int applyConvolutionColor(png_byte **rows, int kernal[3][3], int x,
        int y, int rgb)
{
    int acc = 0;
    png_byte *pxl;

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            pxl = getPixel(rows, i + y, j + x);
            acc += pxl[rgb] * kernal[i][j];
        }
    }

    return acc;
}

/**
 * Not sure how correct this is but the effects are pretty interesting.
 *
 * For each color chanel apply a convolution
 */
static void sobelEdgeDetectionColor(int width, int height, png_byte **inrows,
        imgEdge *ie)
{
    png_byte *pxl;
    png_byte *pxlgx;
    png_byte *pxlgy;
    png_byte *pxlorig;

    for (int y = 0; y < height - 2; ++y) {
        for (int x = 0; x < width - 2; ++x) {
            pxl = getPixel(ie->rows, y, x);
            pxlgx = getPixel(ie->gx, y, x);
            pxlgy = getPixel(ie->gy, y, x);
            pxlorig = getPixel(inrows, y, x);

            pxlgx[R] = applyConvolutionColor(inrows, sobelMX, x, y, R);
            pxlgx[G] = applyConvolutionColor(inrows, sobelMX, x, y, G);
            pxlgx[B] = applyConvolutionColor(inrows, sobelMX, x, y, B);

            pxlgy[R] = applyConvolutionColor(inrows, sobelMY, x, y, R);
            pxlgy[G] = applyConvolutionColor(inrows, sobelMY, x, y, G);
            pxlgy[B] = applyConvolutionColor(inrows, sobelMY, x, y, B);

            pxl[R] = (int)sqrt(pxlgx[R] * pxlgx[R] + pxlgy[R] + pxlgy[R]);
            pxl[G] = (int)sqrt(pxlgx[G] * pxlgx[G] + pxlgy[G] + pxlgy[G]);
            pxl[B] = (int)sqrt(pxlgx[B] * pxlgx[B] + pxlgy[B] + pxlgy[B]);

            pxl[A] = pxlorig[A];
            pxlgy[A] = pxlorig[A];
            pxlgx[A] = pxlorig[A];
        }
    }
}

/* image must be greyscale BEFORE putting through this algorithm */
static void sobelEdgeDetectionGreyscale(int width, int height,
        png_byte **inrows, imgEdge *ie)
{
    int gx;
    int gy;
    png_byte *pxl;
    png_byte *pxlgx;
    png_byte *pxlgy;
    png_byte *pxlorig;

    for (int y = 0; y < height - 2; ++y) {
        for (int x = 0; x < width - 2; ++x) {
            pxl = getPixel(ie->rows, y, x);
            pxlgx = getPixel(ie->gx, y, x);
            pxlgy = getPixel(ie->gy, y, x);
            pxlorig = getPixel(inrows, y, x);

            gx = applyConvolutionGreyscale(inrows, sobelMX, x, y);
            gy = applyConvolutionGreyscale(inrows, sobelMY, x, y);

            setGreyscalePixel(pxl, sqrt(gx * gx + gy * gy));
            setGreyscalePixel(pxlgx, gx);
            setGreyscalePixel(pxlgy, gy);

            pxl[A] = pxlorig[A];
            pxlgy[A] = pxlorig[A];
            pxlgx[A] = pxlorig[A];
        }
    }
}

/**
 * Pick an edgeDetection algorithm based on flags
 */
void sobelEdgeDetection(int width, int height, png_byte **inrows, imgEdge *ie,
        int flags)
{
    if (flags & IMG_GREYSCALE)
        sobelEdgeDetectionGreyscale(width, height, inrows, ie);
    else if (flags & IMG_COLOR)
        sobelEdgeDetectionColor(width, height, inrows, ie);
}

static void minMaxNoramlisationColor(int width, int height, png_byte **rows) {
    int minR = 1000000;
    int minG = 1000000;
    int minB = 1000000;

    int maxR = 0;
    int maxG = 0;
    int maxB = 0;

    int curR = 0;
    int curG = 0;
    int curB = 0;

    png_byte *px;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            px = getPixel(rows, y, x);

            curR = px[R];
            curG = px[G];
            curB = px[B];

            if (curR < minR)
                minR = curR;
            if (curR > maxR)
                maxR = curR;

            if (curG < minG)
                minG = curG;
            if (curG > maxG)
                maxG = curG;

            if (curB < minB)
                minB = curB;
            if (curB > maxB)
                maxB = curB;
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            px = getPixel(rows, y, x);

            px[R] = (int)(((int)(px[R] - minR) / (int)(maxR - minR)) * 255);
            px[G] = (int)(((int)(px[G] - minG) / (int)(maxG - minG)) * 255);
            px[B] = (int)(((int)(px[B] - minB) / (int)(maxB - minB)) * 255);
        }
    }
}

static void minMaxNoramlisationGreyscale(int width, int height,
        png_byte **rows)
{
    int min = 1000000;
    int max = 0;
    int cur = 0;
    png_byte *px;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            px = getPixel(rows, y, x);
            cur = getGreyscalePixel(px);

            if (cur < min)
                min = cur;
            else if (cur > max)
                max = cur;
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            px = getPixel(rows, y, x);
            cur = getGreyscalePixel(px);
            setGreyscalePixel(px, ((cur - min) / (max - min)) * 255);
        }
    }
}

void minMaxNoramlisation(int width, int height, png_byte **rows, int flags) {
    if (flags & IMG_GREYSCALE)
        minMaxNoramlisationGreyscale(width, height, rows);
    else if (flags & IMG_COLOR)
        minMaxNoramlisationColor(width, height, rows);
}
