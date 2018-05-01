#ifndef LIST_H
#define LIST_H

typedef enum {
  STRING,
  INT
} TYPE;

typedef enum {
  SMALL,
  MEDIUM,
  LARGE
} INCREASESIZE;

typedef struct List {
  void ** array;
  size_t length;
  size_t slots;
  TYPE type;
  INCREASESIZE increaseSize;
} List;

void listInit(List * list, size_t initialSize, INCREASESIZE increaseSize, TYPE type);
void listAppend(List * list, void * element);
void * listGet(List * list, int idx);
void listFree(List * list);

#endif
