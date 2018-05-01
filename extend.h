#ifndef EXTEND_H
#define EXTEND_H

#include "list.h"

const char * degens[] = {
  "AC",   // M
  "AG",   // R
  "AT",   // W
  "CG",   // S
  "CT",   // Y
  "GT",   // K
  "ACG",  // V
  "ACT",  // H
  "AGT",  // D
  "CGT",  // B
  "ACGT"  // N
};


void extend(char * seq, List * out);
int numExtendedSeqs(char * seq);

#endif
