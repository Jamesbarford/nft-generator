#ifndef LIST_H
#define LIST_H

#define DLLIST_FAILURE -1
#define DLLIST_SUCCESS  0

#define DLLIST_ITER_NEXT 1
#define DLLIST_ITER_PREV 2

struct dllist;
struct dllistNode;

typedef struct dllistNode {
	struct dllistNode *next;
	struct dllistNode *previous;
	void *data;
} dllistNode;

typedef struct dllistIterator {
	dllistNode *next;
	int dir;
} dllistIterator;

typedef struct dllist {
	dllistNode *head;
	void (*free)(void *);
	int (*match)(void *, void *);
	unsigned int size;
} dllist;

#define getDLListNodeValue(x) ((x)->data)
#define setDLListFree(x, y)   ((x)->free = (y))
#define setDLListMatch(x, y)  ((x)->match = (y))

dllist *dllistCreate();
dllistIterator *dllistCreateIterator(dllist *list, int dir);
int dllistFilterList(dllist *list, int(*predicate)(void *));

int dllistAddNodeStart(dllist *list, void *data);
int dllistAddNodeEnd(dllist *list, void *data);
int dllistInsertAfterMatch(dllist *list, void *data, void *needle);
int dllistDeleteNode(dllist *, void *needle);
dllistNode *dllistGet(dllist *, int);

void dllistPrint(dllist *list);

#endif
