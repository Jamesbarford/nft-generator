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
#ifndef __HMAP_H__
#define __HMAP_H__

/**
 * A non-resizable hashtable loosely based on the K&R hashtab
 */

struct hmapEntry;

typedef void hmapFreeValue(void *);

typedef struct hmapEntry {
    char *key;
    void *value;
    unsigned int hash;
    struct hmapEntry *next;
} hmapEntry;

typedef struct hmap {
    unsigned int size;
    unsigned int capacity;
    int mask;
    hmapFreeValue *freeValue;
    hmapEntry **entries;
} hmap;

typedef struct hmapIterator {
    unsigned int idx;
    hmap *hm;
    hmapEntry *cur;
} hmapIterator;

void hmapReleaseEntries(hmap *hm);
void hmapClear(hmap *hm);
void hmapRelease(hmap *hm);

hmap *hmapCreate(int capacity);
hmapEntry *hmapGetValue(hmap *hm, char *key);
int hmapSetValue(hmap *hm, char *key, void *value);
int hmapGetNext(hmapIterator *iter);
hmapIterator *hmapCreateIterator(hmap *hm);
void hmapReleaseIterator(hmapIterator *iter);

#endif
