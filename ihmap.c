#include <stdlib.h>

#include "ihmap.h"

void ihmapReleaseEntries(ihmap *hm) {
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

void ihmapClear(ihmap *hm) {
	ihmapReleaseEntries(hm);
	hm->entries = calloc(hm->capacity, sizeof(hmapEntry));
}

unsigned int hashKey(int x) {
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;
	return x;
}

void ihmapRelease(ihmap *hm) {
	if (hm) {
		ihmapReleaseEntries(hm);
		free(hm->entries);
		free(hm);
	}
}

ihmap *ihmapCreate(int capacity) {
	ihmap *hm;

	if ((hm = malloc(sizeof(ihmap))) == NULL)
		return NULL;

	hm->size = 0;
	hm->capacity = capacity;
	hm->mask = capacity - 1;
	hm->entries = calloc(capacity, sizeof(hmapEntry));
	return hm;
} 

hmapEntry *ihmapGetValue(ihmap *hm, int key) {
	hmapEntry *he;
	unsigned int hash = hashKey(key);
	unsigned int idx = hash & hm->mask;
	he = hm->entries[idx];

	while (he) {
		if (he->key == key) {
			return he;
		}
		he = he->next;
	}

	return NULL;
}

int ihmapSetValue(ihmap *hm, int key, int value) {
	unsigned int hash = hashKey(key);
	unsigned int idx;
	hmapEntry *he;

	if ((he = ihmapGetValue(hm, key)) != NULL) {
		he->value = value;
		return 1;
	}

	if (he == NULL) {
		if ((he = malloc(sizeof(hmapEntry))) == NULL)
			return -1;
		idx = hash & hm->mask;
		he->key = key;
		he->value = value;
		he->next = hm->entries[idx];
		hm->entries[idx] = he;
		hm->size++;
	}

	return 1;
}

int ihmapGetNext(ihmapIterator *iter) {
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

ihmapIterator *ihmapCreateIterator(ihmap *hm) {
	ihmapIterator *iter;

	if ((iter = (ihmapIterator *)malloc(sizeof(ihmapIterator))) == NULL)
		return NULL;

	iter->idx = 0;
	iter->hm = hm;
	iter->cur = NULL;

	return iter;
}

void ihmapReleaseIterator(ihmapIterator *iter) {
	if (iter) {
		free(iter);
	}
}
