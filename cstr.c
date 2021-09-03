#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "cstr.h"

cstrArray *cstrArrayCreate() {
    cstrArray *ca;

    if ((ca = (cstrArray *)malloc(sizeof(cstrArray))) == NULL)
        return NULL;

    ca->size = 0;
    ca->arr = malloc(1);

    return ca;
}

void cstrArrayRelease(cstrArray *ca) {
    if (ca) {
        for (int i = 0; i < ca->size; ++i) {
            cstrRelease(ca->arr[i]);
        }
        free(ca->arr);
        free(ca);
    }
}

cstr *cstrCreate(char *str) {
    cstr *c = cstrEmpty();
    c->len =strlen(str);
    c->buf = strdup(str);

    return c;
}

void cstrRelease(cstr *c) {
    if (c) {
        free(c->buf);
        free(c);
    }
}

cstr *cstrEmpty() {
    cstr *c;

    if ((c = (cstr *)malloc(sizeof(cstr))) == NULL)
        return NULL;

    c->len = 0;
    c->buf = malloc(1);

    return c;
}

cstrArray *cstrSplit(char delimiter, char *str) {
    cstrArray *ca = cstrArrayCreate();
    int i = 0;
    char temp[BUFSIZ];
    char *ptr = str;

    while (*ptr != '\0') {
        if (*ptr == delimiter) {
            temp[i] = '\0';
            ca->arr = (cstr **)realloc(ca->arr, sizeof(cstr) * ca->size + 1);
            ca->arr[ca->size] = cstrCreate(temp);
            ca->size++;
            memset(temp, 0, BUFSIZ);
            ptr++;
            i = 0;
            continue;
        }

        temp[i++] = *ptr;
        ptr++;
    }

    ca->arr = (cstr **)realloc(ca->arr, sizeof(cstr) * (ca->size + 1));
    ca->arr[ca->size] = cstrCreate(temp);
    ca->size++;

    return ca;
}

void cstrArrayPrint(cstrArray *ca) {
    printf("[ ");
    for (int i = 0; i < ca->size; ++i) {
        if (i + 1 != ca->size) {
            printf("%s, ", ca->arr[i]->buf);
        } else {
            printf("%s", ca->arr[i]->buf);
        }
    }
    printf(" ]\n");
}
