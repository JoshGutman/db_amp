#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

#include "getfastas.h"
#include "search.h"
//#include "extend.h"
#include "list.h"

int main(int argc, char * argv[]) {
  char * forward = NULL;
  char * reverse = NULL;
  char * input = NULL;
  char * output = NULL;

  int c;

  while((c = getopt(argc, argv, "f:r:i:o:")) != -1) {
    switch(c) {
    
    case 'f' :
      forward = optarg;
      break;
    case 'r' :
      reverse = optarg;
      break;
    case 'i' :
      input = optarg;
      break;
    case 'o' :
      output = optarg;
      break;
    case '?' :
      if (optopt == 'c')
	fprintf (stderr, "Option -%c requires an argument.\n", optopt);
      else if (isprint (optopt))
	fprintf (stderr, "Unknown option `-%c'.\n", optopt);
      else
	fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
      return 1;

    default :
      abort ();

    }
  }

  printf("%s\t%s\t%s\t%s\n", forward, reverse, input, output);

  List fastas = getFastas(input);
  int i;
  for (i = 0; i < fastas.length; i++) {
    printf("%s\n", (char *) listGet(&fastas, i));
  }


  return 0;
}
