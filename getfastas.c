#define _XOPEN_SOURCE 500 
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <linux/limits.h>

#include "getfastas.h"
//#include "list.h"

static const char EXTENSION[] = ".fasta";
static List LIST;

static int display_info(const char *fpath, const struct stat *sb,
                        int tflag, struct FTW *ftwbuf) {
  if (tflag == FTW_F) {
    char * resolvedPath = (char *)malloc(PATH_MAX/4);
    realpath(fpath + ftwbuf->base, resolvedPath);
    char * dot = strrchr(resolvedPath, '.');
    if (dot && !strcmp(dot, EXTENSION)) {
      listAppend(&LIST, (void *)resolvedPath);
    }
  }
  return 0;
}


List getFastas(char * directory) {
  listInit(&LIST, 1, LARGE, STRING);
  int flags = 0;
  flags |= FTW_CHDIR;

  if (nftw(directory, display_info, 20, flags) == -1) {
      perror("nftw");
      exit(EXIT_FAILURE);
  }
  return LIST;
}

/*
int main(int argc, char ** argv) {
  List x;
  char * directory = argv[1];
  x = getFastas(directory);
  int i;
  for (i = 0; i < x.length; i++) {
    printf("%s\n", (char *)listGet(&x, i));
  }
  return 0;
}
*/
