/* the worlds worst hashtable */
#ifndef __INT_HMAP__
#define __INT_HMAP__

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
