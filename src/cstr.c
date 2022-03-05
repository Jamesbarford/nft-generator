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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "cstr.h"

void cstrRelease(void *str) {
    if (str) {
        cstr *s = str;
        s -= CSTR_PAD;
        s[4] = 'p';
        free(s);
    }
} 

void cstrArrayRelease(cstr **arr, int count) {
    if (arr) {
        for (int i = 0; i < count; ++i)
            cstrRelease(arr[i]);
        free(arr);
    }
} 

void cstrSetLen(cstr *str, int len) {
    str -= CSTR_PAD;
    memcpy(str, &len, sizeof(int));
    (str)[4] = '\0';
    str += CSTR_PAD;
}

/* Get the integer out of the string */
int cstrlen(cstr *str) {
    int len;
    str -= CSTR_PAD;
    len = *(int *)str;
    str += CSTR_PAD;
    return len;
}

int cstrToString(cstr *str, char *outbuf, int outbuflen) {
    int len = cstrlen(str);
    if (outbuflen < len)
        return CSTR_ERR;

    memcpy(outbuf, str, len);
    outbuf[len] = '\0';

    return CSTR_OK;
}

/**
 * The first 5 bytes are reserved. 4 for an integer, with the 5th being
 * a null terminator.
 * Resultant buffer looks like this:
 *   LEN     LEN      LEN     LEN    END   STRING    END
 * ['0xFF', '0xFF', '0xFF', '0xFF', '\0', 'a', 'b', '\0'];
 *
 * With the pointer moved forward twice to the start of the string. OR all
 * of the LEN properties together to get the length of the string;
 */
cstr *cstrEmpty(int len) {
    cstr *out;

    if ((out = calloc(sizeof(char), len + 1 + CSTR_PAD)) == NULL)
        return NULL;

    out += CSTR_PAD;
    return out;
}

cstr *cstrCreate(char *original, int len) {
    cstr *outbuf;

    if ((outbuf = cstrEmpty(len)) == NULL)
        return NULL;

    cstrSetLen(outbuf, len);
    memcpy(outbuf, original, len);
    outbuf[len] = '\0';
    return outbuf;
}

cstr *cstrdup(cstr *original) {
    cstr *duped;
    int len;

    len = cstrlen(original);

    if ((duped = cstrEmpty(len)) == NULL)
        return NULL;

    cstrSetLen(duped, len);
    memcpy(duped, original, len);

    return duped;
}

/* this feels better than trying to guess the length of what we want */
cstr *cstrCopyUntil(char *original, char terminator) {
    int i;
    char *ptr;
    cstr *str;

    ptr = original;
    i = 0;

    // figure out how long the string is
    while (*ptr != terminator && *ptr != '\0') { 
        i++;
        ptr++;
    }

    if ((str = cstrEmpty(i)) == NULL) return NULL;
    memcpy(str, original, i);
    cstrSetLen(str, i);

    str[i] = '\0';

    return str;
}

/**
 * Returns the position in the string where the first match begins
 *
 * Example:
 * "the_best_string_ever"
 * the    -> 0
 * best   -> 4
 * string -> 9
 * ever   -> 16
 *
 * or -1 if there is no match
 *
 */
int cstrIndexOf(cstr *str, char *pattern) {
    int i, j;
    char c, *ptr;

    for (i = 0; i < cstrlen(str); ++i) {
        c = str[i];

        if (c == pattern[0]) {
            for (ptr = pattern, j = i; *ptr != 0; ptr++)
                if (*ptr != str[j++]) break;
            if (*ptr == '\0') return i;
        }
    }

    return -1;
}

/**
 * Split into cstrings on delimiter
 */
cstr **cstrSplit(char *to_split, char delimiter, int *count) {
    cstr **outArr, *ptr;
    int i;
    char tmp[BUFSIZ];

    if (*to_split == delimiter)
        to_split++;

    ptr = to_split;
    *count = 0;
    i = 0;

    if ((outArr = malloc(sizeof(char) * 1)) == NULL)
        return NULL;

    while (*ptr != '\0') {
        if (*ptr == delimiter) {
            tmp[i] = '\0';
            outArr = (char **)realloc(outArr, sizeof(char) * (*count + 1));
            outArr[*count] = cstrCreate(tmp, i);

            i = 0;
            ptr++;
            (*count)++;
            memset(tmp, 0, BUFSIZ);
            continue;
        }

        tmp[i++] = *ptr;
        ptr++;
    }

    tmp[i] = '\0';
    outArr[*count] = cstrCreate(tmp, i);
    (*count)++;

    return outArr;
}

/**
 * sort of casts a char* to an array of cstr**
 */
cstr **cstrCastArray(char *original) {
    cstr *str;
    cstr **array;

    if ((array = malloc(sizeof(cstr *))) == NULL)
        return NULL;

    if ((str = cstrCreate(original, strlen(original))) == NULL) {
        free(array);
        return NULL;
    }
    
    array[0] = str;

    return array;
}
