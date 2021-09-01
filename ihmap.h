#ifndef __INT_HMAP__
#define __INT_HMAP__

struct hmapEntry;

typedef struct hmapEntry {
	int key;
	int value;
	struct hmapEntry *next;
} hmapEntry;

typedef struct ihmap {
	unsigned int size;
	unsigned int capacity;
	int mask;
	hmapEntry **entries;
} ihmap;

typedef struct ihmapIterator {
	unsigned int idx;
	ihmap *hm;
	hmapEntry *cur;
} ihmapIterator;

void ihmapReleaseEntries(ihmap *hm); 
void ihmapClear(ihmap *hm);
void ihmapRelease(ihmap *hm);

ihmap *ihmapCreate(int capacity);
hmapEntry *ihmapGetValue(ihmap *hm, int key);
int ihmapSetValue(ihmap *hm, int key, int value);
int ihmapGetNext(ihmapIterator *iter);
ihmapIterator *ihmapCreateIterator(ihmap *hm);
void ihmapReleaseIterator(ihmapIterator *iter);


#endif
