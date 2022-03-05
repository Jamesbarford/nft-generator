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
#ifndef __IMG_PNG_H__
#define __IMG_PNG_H__

#include <png.h>

#define R 0
#define G 1
#define B 2
#define A 3

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

typedef struct imgEdge {
    int width;
    int height;
    png_byte **rows;
    png_byte **gx;
    png_byte **gy;
} imgEdge;

void printPixel(int x, int y, png_byte *pixel);
int imgpngAllocRows(imgpng *img);

imgpng *imgpngCreate(void);
imgpng *imgpngCreateFromFile(char *file_name);

void imgpngRelease(imgpng *img);
imgpngBasic *imgpngBasicCreate(int width, int height);
imgEdge *imgEdgeCreate(imgpng *img);
void imgEdgeRelease(imgEdge *ie);

void imgpngBasicRelease(imgpngBasic *imgb);
void imgWriteToFile(int width, int height, png_byte **rows, png_byte bitdepth,
                    png_byte colortype, char *file_name);
void colourCheck(imgpng *img);
png_byte **pngAllocRows(png_struct *png_ptr, png_info *info, int height);
void imgpngRowsRelease(int height, png_byte **rows);
imgpngBasic *imgpngDuplicate(imgpng *img);

#endif
