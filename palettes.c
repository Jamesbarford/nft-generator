#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "palettes.h"
#include "hmap.h"

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

	rgbArray[0] = (hexTable(hex[1]) << 4) | hexTable(hex[2]);
	rgbArray[1] = (hexTable(hex[3]) << 4) | hexTable(hex[4]);
	rgbArray[2] = (hexTable(hex[5]) << 4) | hexTable(hex[6]);
	return 1;
}
static int palette1[][3] = {
  {13, 43, 69},
  {32, 60, 86},
  {84, 78, 104},
  {141, 105, 122},
  {208, 129, 89},
  {255, 170, 94},
  {255, 212, 163},
  {255, 236, 214},
};

static int palette2[][3] = {
  {48, 0, 48},
  {96, 40, 120},
  {248, 144, 32},
  {248, 240, 136},
};

static int palette3[][3] = {
  {26, 28, 44},
  {93, 39, 93},
  {177, 62, 83},
  {239, 125, 87},
  {255, 205, 117},
  {167, 240, 112},
  {56, 183, 100},
  {37, 113, 121},
  {41, 54, 111},
  {59, 93, 201},
  {65, 166, 246},
  {115, 239, 247},
  {244, 244, 244},
  {148, 176, 194},
  {86, 108, 134},
  {51, 60, 87},
};

static int palette4[][3] = {
  {44, 33, 55},
  {118, 68, 98},
  {237, 180, 161},
  {169, 104, 104},
};

static int palette5[][3] = {
  {7, 5, 5},
  {33, 25, 25},
  {82, 58, 42},
  {138, 107, 62},
  {193, 156, 77},
  {234, 219, 116},
  {160, 179, 53},
  {83, 124, 68},
  {66, 60, 86},
  {89, 111, 175},
  {107, 185, 182},
  {251, 250, 249},
  {184, 170, 176},
  {121, 112, 126},
  {148, 91, 40},
};

static int palette6[][3] = {
  {140, 143, 174},
  {88, 69, 99},
  {62, 33, 55},
  {154, 99, 72},
  {215, 155, 125},
  {245, 237, 186},
  {192, 199, 65},
  {100, 125, 52},
  {228, 148, 58},
  {157, 48, 59},
  {210, 100, 113},
  {112, 55, 127},
  {126, 196, 193},
  {52, 133, 157},
  {23, 67, 75},
  {31, 14, 28},
};

static int palette7[][3] = {
  {94, 96, 110},
  {34, 52, 209},
  {12, 126, 69},
  {68, 170, 204},
  {138, 54, 34},
  {235, 138, 96},
  {0, 0, 0},
  {92, 46, 120},
  {226, 61, 105},
  {170, 92, 61},
  {255, 217, 63},
  {181, 181, 181},
  {255, 255, 255},
};

static int palette8[][3] = {
  {21, 25, 26},
  {138, 76, 88},
  {217, 98, 117},
  {230, 184, 193},
  {69, 107, 115},
  {75, 151, 166},
  {165, 189, 194},
  {255, 245, 247},
};

static colorPalette *allocPalette(int size, int palette[][3]) {
  colorPalette *p;

  if ((p = malloc(sizeof(colorPalette))) == NULL)
    return NULL;

  p->size = size;
  p->colors = (int **)malloc(sizeof(int *));

  for (int i = 0; i < size; ++i) {
    p->colors[i] = malloc(sizeof(int) * 3);
    p->colors[i][0] = palette[i][0];
    p->colors[i][1] = palette[i][1];
    p->colors[i][2] = palette[i][2];
  }

  return p;
}

hmap *colorPaletteCreate() {
  hmap *hm = hmapCreate(1 << 5);

	hmapSetValue(hm, "1", allocPalette(8, palette1));
	hmapSetValue(hm, "2", allocPalette(4, palette2));
  hmapSetValue(hm, "3", allocPalette(16, palette3));
	hmapSetValue(hm, "4", allocPalette(4, palette4));
	hmapSetValue(hm, "5", allocPalette(15, palette5));
	hmapSetValue(hm, "6", allocPalette(16, palette6));
	hmapSetValue(hm, "7", allocPalette(13, palette7));
	hmapSetValue(hm, "8", allocPalette(8, palette8));

  return hm;
}
