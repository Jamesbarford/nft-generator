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

#include <stdlib.h>
#include <string.h>

#include "hmap.h"

void hmapReleaseEntries(hmap *hm) {
    hmapEntry *he;
    hmapEntry *next;

    for (unsigned int i = 0; i < hm->capacity; ++i) {
        next = hm->entries[i];
        while (next) {
            he = next;
            next = he->next;
            if (hm->freeValue)
                hm->freeValue(he->value);
            free(he);
        }
    }
}

void hmapClear(hmap *hm) {
    hmapReleaseEntries(hm);
    hm->entries = calloc(hm->capacity, sizeof(hmapEntry));
}

void hmapRelease(hmap *hm) {
    if (hm) {
        hmapReleaseEntries(hm);
        free(hm->entries);
        free(hm);
    }
}

/**
 * capacity MUST be a power of 2 else the behaviour is undefined
 */
hmap *hmapCreate(int capacity) {
    hmap *hm;

    if ((hm = malloc(sizeof(hmap))) == NULL)
        return NULL;

    hm->size = 0;
    hm->capacity = capacity;
    hm->mask = capacity - 1;
    hm->entries = calloc(capacity, sizeof(hmapEntry));
    return hm;
}

static inline unsigned int hashKey(const char *s) {
    unsigned int h = (unsigned int)*s;
    if (h) {
        for (++s; *s; ++s) {
            h = (h << 5) - h + (unsigned int)*s;
        }
    }
    return h;
}

/* compare hashes then keys */
static inline int keyCompare(int h1, char *k1, int h2, char *k2) {
    return h1 == h2 && (strcmp(k1, k2) == 0);
}

hmapEntry *hmapGetValue(hmap *hm, char *key) {
    hmapEntry *he;
    unsigned int hash = hashKey(key);
    unsigned int idx = hash & hm->mask;
    he = hm->entries[idx];

    while (he) {
        if (keyCompare(hash, key, he->hash, he->key)) {
            return he;
        }
        he = he->next;
    }

    return NULL;
}

int hmapSetValue(hmap *hm, char *key, void *value) {
    unsigned int hash = hashKey(key);
    unsigned int idx;
    hmapEntry *he;

    if ((he = hmapGetValue(hm, key)) != NULL) {
        he->value = value;
        return 1;
    }

    if (he == NULL) {
        if ((he = malloc(sizeof(hmapEntry))) == NULL)
            return -1;
        idx = hash & hm->mask;
        he->key = key;
        he->value = value;
        he->hash = hash;
        he->next = hm->entries[idx];
        hm->entries[idx] = he;
        hm->size++;
    }

    return 1;
}

int hmapGetNext(hmapIterator *iter) {
    unsigned int idx;

    while (iter->idx < iter->hm->capacity) {
        if (iter->cur == NULL || iter->cur->next == NULL) {
            idx = iter->idx;
            iter->idx++;
            if (iter->hm->entries[idx] != NULL) {
                iter->cur = iter->hm->entries[idx];
                return 1;
            }
        } else if (iter->cur->next != NULL) {
            iter->cur = iter->cur->next;
            return 1;
        } else {
            return 0;
        }
    }
    return 0;
}

hmapIterator *hmapCreateIterator(hmap *hm) {
    hmapIterator *iter;

    if ((iter = (hmapIterator *)malloc(sizeof(hmapIterator))) == NULL)
        return NULL;

    iter->idx = 0;
    iter->hm = hm;
    iter->cur = NULL;

    return iter;
}

void hmapReleaseIterator(hmapIterator *iter) {
    if (iter) {
        free(iter);
    }
}
