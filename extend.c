#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "extend.h"
//#include "list.h"

void extend(char * seq, List * out) {
  int notDegenerate = 1;
  int i;
  int j;
  int degenIdx;

  for (i = 0; i < strlen(seq); i++) {
    switch(seq[i]) {
    
    case 'M' :
      notDegenerate = 0;
      degenIdx = 0;
      break;
    case 'R' :
      notDegenerate = 0;
      degenIdx = 1;
      break;
    case 'W' :
      notDegenerate = 0;
      degenIdx = 2;
      break;
    case 'S' :
      notDegenerate = 0;
      degenIdx = 3;
      break;
    case 'Y' :
      notDegenerate = 0;
      degenIdx = 4;
      break;
    case 'K' :
      notDegenerate = 0;
      degenIdx = 5;
      break;
    case 'V' :
      notDegenerate = 0;
      degenIdx = 6;
      break;
    case 'H' :
      notDegenerate = 0;
      degenIdx = 7;
      break;
    case 'D' :
      notDegenerate = 0;
      degenIdx = 8;
      break;
    case 'B' :
      notDegenerate = 0;
      degenIdx = 9;
      break;
    case 'N' :
      notDegenerate = 0;
      degenIdx = 10;
      break;

    default :
      continue;
    }
    
    for (j = 0; j < strlen(degens[degenIdx]); j++) {
      char * newSeq = malloc(sizeof(char) * (strlen(seq) + 1));
      strcpy(newSeq, seq);
      newSeq[i] = degens[degenIdx][j];
      extend(newSeq, out);
    }

    break;
  }

  if (notDegenerate) {
    listAppend(out, (void *)seq);
  }

}


int numExtendedSeqs(char * seq) {
  int i;
  int degenIdx;
  int out = 1;

  for (i = 0; i < strlen(seq); i++) {
    switch(seq[i]) {

    case 'M' :
      degenIdx = 0;
      break;
    case 'R' :
      degenIdx = 1;
      break;
    case 'W' :
      degenIdx = 2;
      break;
    case 'S' :
      degenIdx = 3;
      break;
    case 'Y' :
      degenIdx = 4;
      break;
    case 'K' :
      degenIdx = 5;
      break;
    case 'V' :
      degenIdx = 6;
      break;
    case 'H' :
      degenIdx = 7;
      break;
    case 'D' :
      degenIdx = 8;
      break;
    case 'B' :
      degenIdx = 9;
      break;
    case 'N' :
      degenIdx = 10;
      break;

    default :
      continue;

    }
    out *= strlen(degens[degenIdx]);
  }

  return out;
}


char * reverseComplement(char * seq) {
  int i;
  int j=0;
  char * out;
  out = malloc(sizeof(char) * (strlen(seq) - 1));

  for (i = strlen(seq)-1; i >= 0; i--) {
    
    switch(seq[i]) {
      
    case 'A' :
      out[j++] = 'T';
      break;
    case 'C' :
      out[j++] = 'G';
      break;
    case 'G' :
      out[j++] = 'C';
      break;
    case 'T' :
      out[j++] = 'A';
      break;
    case 'R' :
      out[j++] = 'Y';
      break;

    case 'Y' :
      out[j++] = 'R';
      break;
    case 'S' :
      out[j++] = 'S';
      break;
    case 'W' :
      out[j++] = 'W';
      break;
    case 'K' :
      out[j++] = 'M';
      break;
    case 'M' :
      out[j++] = 'K';
      break;

    case 'B' :
      out[j++] = 'V';
      break;
    case 'V' :
      out[j++] = 'B';
      break;
    case 'D' :
      out[j++] = 'H';
      break;
    case 'H' :
      out[j++] = 'D';
      break;
    case 'N' :
      out[j++] = 'N';
      break;

    default :
      out[j++] = seq[i];
      continue;
    }

  }

  return out;
}




/*
int main() {
  char test[] = "RR";
  char test2[] = "NAN";
  
  printf("%d, %d\n", numExtendedSeqs(test), numExtendedSeqs(test2));

  printf("%s\n", reverseComplement(test2));

  List l;
  listInit(&l, 16, SMALL, STRING);
  
  extend(test2, &l);

  int i;
  for (i = 0; i < l.length; i++) {
    printf("%s\n", (char *) listGet(&l, i));
  }

  return 0;
}
*/
