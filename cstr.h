#ifndef __CSTR_H__
#define __CSTR_H__

typedef struct cstr {
    int len;
    char *buf;
} cstr;

typedef struct cstrArray {
    int size;
    cstr **arr;
} cstrArray;

cstrArray *cstrArrayCreate();
void cstrArrayRelease(cstrArray *ca);
cstrArray *cstrSplit(char delimiter, char *str);
void cstrArrayPrint(cstrArray *ca);

cstr *cstrCreate(char *str);
void cstrRelease(cstr *c);
cstr *cstrEmpty();

#endif
