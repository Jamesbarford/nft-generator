#include <stdlib.h>
#include <stdio.h>

#include "list.h"

static int listPanic(char *msg) {
	fprintf(stderr, "%s\n", msg);
	return DLLIST_FAILURE;
}

static dllistNode *dllistCreateNode(void *data) {
	dllistNode *node = NULL;
	if ((node = (dllistNode *)malloc(sizeof(dllistNode))) == NULL)
		return NULL;

	node->data = data;
	node->next = NULL;
	node->previous = NULL;

	return node;
}

dllist *dllistCreate() {
	dllist *list = NULL;

	if ((list = (dllist *)malloc(sizeof(dllist))) == NULL)
		return NULL;

	list->head = NULL;
	list->size = 0;
	list->free = NULL;
	list->match = NULL;

	return list;
}

/**
 * This is just for use when the head is NULL
 */
static int _replaceHead(dllist *list, dllistNode *node) {
	node->next = node;
	node->previous = node;
	list->head = node;
	list->size++;
	return DLLIST_SUCCESS;
}

static void _safeFreeListNode(dllist *list, dllistNode *node) {
	if (list->free) {
		list->free(node->data);
	}
	node->next = NULL;
	node->previous = NULL;
	(void)free(node);
}

int dllistAddNodeEnd(dllist *list, void *data) {
	dllistNode *node = dllistCreateNode(data);
	if (node == NULL)
		return DLLIST_FAILURE;

	if (list->head == NULL)
		return _replaceHead(list, node);

	else {
		dllistNode *end = list->head->previous;
		node->next = list->head;
		list->head->previous = node;
		node->previous = end;
		end->next = node;
	}

	list->size++;
	return DLLIST_SUCCESS;
}

/**
 * Create a new head with the created node
 */
int dllistAddNodeStart(dllist *list, void *data) {
	dllistNode *node = dllistCreateNode(data);
	if (node == NULL)
		return DLLIST_FAILURE;

	if (list->head == NULL)
		return _replaceHead(list, node);

	else {
		dllistNode *end = list->head->previous;
		node->next = list->head;
		node->previous = end;
		end->next = node;
		list->head->previous = node;
		list->head = node;
	}

	list->size++;
	return DLLIST_SUCCESS;
}

int dllistInsertAfterMatch(dllist *list, void *data, void *needle) {
	if (list->match == NULL)
		return listPanic("list->match is NULL, cannot complete insertAfterMatch");

	dllistNode *node = dllistCreateNode(data);
	if (node == NULL)
		return DLLIST_FAILURE;
	if (list->head == NULL)
		return DLLIST_FAILURE;

	dllistNode *tmp = list->head;

	while (list->match(tmp->data, needle) != 0) {
		tmp = tmp->next;
		// there was no match so abort
		if (tmp == list->head)
			return DLLIST_FAILURE;
	}

	dllistNode *nxt = tmp->next;
	tmp->next = node;
	node->previous = tmp;
	node->next = nxt;
	nxt->previous = node;

	return DLLIST_SUCCESS;
}


dllistNode *dllistGet(dllist *list, int idx) {
	int i = 0;
	dllistNode *dn = list->head;

	while (dn && i < idx) {
		dn = dn->next;
		idx++;
	}

	if (idx == i)
		return dn;
	return NULL;
}

int dllListDeleteNode(dllist *list, void *needle) {
	if (list == NULL || list->head == NULL)
		return DLLIST_SUCCESS; // it's empty so, kind of a success?

	if (list->match == NULL)
		return listPanic("list->match is NULL, cannot complete deleteListNode");

	dllistNode *cur = list->head;
	dllistNode *prev = NULL;

	while (list->match(cur->data, needle) != 0) {
		if (cur->next == list->head)
			return DLLIST_FAILURE;

		prev = cur;
		cur = cur->next;
	}

	if (cur->next == list->head && prev == NULL) {
		list->head = NULL;

		_safeFreeListNode(list,  cur);
	}

	if (cur == list->head) {
		prev = list->head->previous;
		list->head = list->head->next;
		prev->next = list->head;
		list->head->previous = prev;

		_safeFreeListNode(list, cur);
	} else if (cur->next == list->head) {
		prev->next = list->head;
		list->head->previous = prev;

		_safeFreeListNode(list, cur);
	} else {
		prev->next = cur->next;
		cur->next->previous  = prev;

		_safeFreeListNode(list, cur);
	}

	list->size--;

	return DLLIST_SUCCESS;
}

void dllistFreeIterator(dllistIterator *iterator) {
	(void)free(iterator);
}

dllistIterator *dllistCreateIterator(dllist *list, int dir) {
	dllistIterator *iterator = NULL;
	if ((iterator = (dllistIterator *)malloc(sizeof(dllistIterator))) == NULL)
		return NULL;

	if (dir == DLLIST_ITER_NEXT)
		iterator->next = list->head;
	else
		iterator->next = list->head->previous;

	iterator->dir = dir;
	return iterator;
}

/**
 * Returns how many items were removed
 */
int dllistFilterList(dllist *list, int(*predicate)(void *)) {
	int filtered = 0;
	dllistIterator *iter = dllistCreateIterator(list, DLLIST_ITER_NEXT);

	while (iter->next != list->head) {
		if (predicate(getDLListNodeValue(iter->next))) {
			_safeFreeListNode(list, iter->next);
			filtered++;
		}
		iter->next = iter->next->next;
	}

	if (predicate(getDLListNodeValue(list->head->previous))) {
		_safeFreeListNode(list, list->head->previous->data);
		filtered++;
	}

	dllistFreeIterator(iter);

	return filtered;
}

void dllistPrint(dllist *list) {
	dllistIterator *iter = dllistCreateIterator(list, DLLIST_ITER_NEXT);

	printf("%s\n", (char *)iter->next->data);
	while (iter->next->next != list->head) {
		printf("%s\n", (char *)getDLListNodeValue(iter->next));
		iter->next = iter->next->next;
	}
}
