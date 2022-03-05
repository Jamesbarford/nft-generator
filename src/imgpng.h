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
