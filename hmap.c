#include <stdlib.h>
#include <string.h>

#include "hmap.h"

void hmapReleaseEntries(hmap *hm) {
	hmapEntry *he;
	hmapEntry *next;

	for (unsigned int i = 0; i < hm->capacity; ++i) {
		he = hm->entries[i];
		if (he != NULL) {
			next = he->next;
			while (next) {
				next = he->next->next;
				free(he->next);
			}
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
		for (++s ; *s; ++s) {
			h = (h << 5) - h + (unsigned int)*s;
		}
	}
	return h;
}

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
