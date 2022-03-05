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
#ifndef __IMAGE_PROCESSING_H__
#define __IMAGE_PROCESSING_H__

#include "imgpng.h"
#include "palettes.h"

#define IMG_GREYSCALE 1
#define IMG_COLOR 2

void imgpngMixChannels(int width, int height, png_byte **rows);
void imgpngMixChannelsCustom(int width, int height, png_byte **rows, int rgb);
void imgpngMixChannelsUntilHeight(int width, int height, png_byte **rows,
        int rgb, int untilHeight);

void imgpngMerge(int width, int height, imgpng **imgs, int imgCount,
                int largest);

void imgpngBasicInit(imgpng *img, imgpngBasic *imgb, int scale);
/**
 * This is quite a simple algorithm and the results are a bit choppy
 */
void pixilateImage(int width, int height, png_byte **rows, int scale);

/**
 * NEW ALGO
 *
 * https://stackoverflow.com/questions/15777821/how-can-i-pixelate-a-jpg-with-java
 */
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
void sobelEdgeDetection(int width, int height, png_byte **inrows, imgEdge *ie,
                        int flags);
void minMaxNoramlisation(int width, int height, png_byte **rows, int flags);

#endif
