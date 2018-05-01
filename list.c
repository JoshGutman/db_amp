#include <stdio.h>
#include <stdlib.h>
#include "list.h"

void listInit(List * list, size_t initialSize, INCREASESIZE increaseSize, TYPE type) {
  list->slots = initialSize;
  list->length = 0;
  list->increaseSize = increaseSize;
  list->type = type;
  list->array = (void **)malloc(initialSize * sizeof(void *));
}

void listAppend(List * list, void * element) {
  if (list->length == list->slots) {
    switch (list->increaseSize) {
    case SMALL:
      list->slots += 5;
    case MEDIUM:
      list->slots += 50;
    case LARGE:
      list->slots += 500;
    }
    list->array = (void **)realloc(list->array, list->slots * sizeof(void *));
  }

  list->array[list->length++] = element;
}

void * listGet(List * list, int idx) {
  return list->array[idx];
}

void listFree(List * list) {
  free(list->array);
  list->array = NULL;
  list->length = list->slots = 0;
}

/*
int main(int argc, char ** argv) {
  List list;
  listInit(&list, 1, SMALL, INT);
  
  int i;
  for (i = 1; i < 5; i++) {
    listAppend(&list, (void *) argv[i]);
  }

  for (i = 0; i < list.length; i++) {
    printf("%d\n", (int)listGet(&list, i));
  }

  return 0;
}
*/
