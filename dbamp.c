#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <mpi.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <omp.h>

#include "getfastas.h"
#include "search.h"
#include "extend.h"
#include "list.h"

int master(int argc, char * argv[], int nprocs);
void masterSend(char * fasta, char * forward, char * reverse, int * isAnti, int reciever, int kill);
void slave(void);
char * readFasta(char * filename);


typedef struct Data {
  char    fasta[PATH_MAX];
  char    forward[100];
  char    reverse[100];
  int     kill;
  int     isAnti;
  char *  amps[10];
} Data;

MPI_Datatype dataMPI;
int myRank;

int main(int argc, char * argv[]) {
  int nprocs;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  
  // Create new MPI datatype
  int count = 6;
  int blockLengths[] = {PATH_MAX, 100, 100, 1, 1, 10};
  MPI_Aint disp[] = {offsetof(Data, fasta),
		     offsetof(Data, forward),
		     offsetof(Data, reverse),
		     offsetof(Data, kill),
		     offsetof(Data, isAnti),
		     offsetof(Data, amps)};
  MPI_Datatype types[] = {MPI_CHAR, MPI_CHAR, MPI_CHAR, MPI_INT, MPI_INT, MPI_CHAR};
  MPI_Type_create_struct(count, blockLengths, disp, types, &dataMPI);
  MPI_Type_commit(&dataMPI);
  

  if (myRank == 0) {
    master(argc, argv, nprocs);
  } else {
    slave();
  }

  MPI_Finalize();
}



int master(int argc, char * argv[], int nprocs) {
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

  List fastas = getFastas(input);

  char * antiForward = reverseComplement(forward);
  char * antiReverse = malloc(sizeof(char) * strlen(reverse) + 1);
  strcpy(antiReverse, reverse);
  reverse = reverseComplement(reverse);
  //char * antiReverse = reverseComplement(reverse);

  int i;
  int reciever = 1;
  int initialIter = (fastas.length*2 < nprocs-1) ? fastas.length*2 : nprocs-1;
  int isAnti = 0;
  for (i = 0; i < initialIter; i++) {
    
    if (isAnti) {
      masterSend((char *)listGet(&fastas, i), antiForward, antiReverse, &isAnti, reciever, 0);
    } else {
      masterSend((char *)listGet(&fastas, i), forward, reverse, &isAnti, reciever, 0);
    }

    reciever++;
    if (reciever >= nprocs) {
      reciever = 1;
    }
  }

  //  printf("Master: Initial sending done...\n");
  //  printf("while i (%d) < fastas.length*2 (%d)...\n", i, fastas.length*2);

  MPI_Status status;
  Data recieved;

  while (i < fastas.length*2) {

    //    printf("Master: Waiting for response...\n");
    MPI_Recv(&recieved, 1, dataMPI, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    //    printf("Master: Response recieved from %d\n", status.MPI_SOURCE);
    int kill = 0;
    if (isAnti) {
      masterSend((char *)listGet(&fastas, i/2), antiForward, antiReverse, &isAnti, status.MPI_SOURCE, kill);
    } else {
      masterSend((char *)listGet(&fastas, i/2), forward, reverse, &isAnti, status.MPI_SOURCE, kill);
    }

    i++;
  }


  for (i = 1; i < nprocs; i++) {
    MPI_Recv(&recieved, 1, dataMPI, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
  }


  for (i = 1; i < nprocs; i++) {
    masterSend("", "", "", &isAnti, i, 1);
  }

  return 0;

}

void masterSend(char * fasta, char * forward, char * reverse, int * isAnti, int reciever, int kill) { 
  int isAntiCpy = *isAnti;
  Data * toSend = (Data *) malloc(sizeof(Data));
  strcpy(toSend->fasta, fasta);
  strcpy(toSend->forward, forward);
  strcpy(toSend->reverse, reverse);
  toSend->isAnti = isAntiCpy;
  toSend->kill = kill;
  //  printf("Master: Sending data to slave rank %d...\n", reciever);
  MPI_Send(toSend, 1, dataMPI, reciever, 0, MPI_COMM_WORLD);
  //  printf("Master: Data successfully sent to slave rank %d\n\n", reciever);
  *isAnti = !(*isAnti);
}


void slave() {
  Data recieved;
  while (1) {
    //    printf("Slave rank %d: Waiting for data from master...\n", myRank);
    MPI_Recv(&recieved, 1, dataMPI, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //    printf("Slave rank %d: Recieved data from master\n", myRank);

    if (recieved.kill) {
      //      printf("Breaking because kill is: %d\n", recieved.kill);
      break;
    }

    List forwards;
    List reverses;
    listInit(&forwards, numExtendedSeqs(recieved.forward), SMALL, STRING);
    listInit(&reverses, numExtendedSeqs(recieved.reverse), SMALL, STRING);

    char * fasta = readFasta(recieved.fasta);
    extend(recieved.forward, &forwards);
    extend(recieved.reverse, &reverses);

    List forwardMatches;
    List reverseMatches;
    listInit(&forwardMatches, 1, SMALL, INT);
    listInit(&reverseMatches, 1, SMALL, INT);
    
    int i;
    int j;
    #pragma omp parallel shared(fasta,forwardMatches,reverseMatches) private(i,j)
      {
      #pragma omp for
      for (i = 0; i < forwards.length; i++) {
	List matches = search(fasta, (char *)listGet(&forwards, i));
	#pragma omp critical
	  {
	  for (j = 0; j < matches.length; j++) {
	    listAppend(&forwardMatches, listGet(&forwards, j));
	  }
	  }
        }

      #pragma omp for
      for (i = 0; i < reverses.length; i++) {
	List matches = search(fasta, (char *)listGet(&reverses, i));
        #pragma omp critical
	{
          for (j = 0; j < matches.length; j++) {
            listAppend(&reverseMatches, listGet(&reverses, j));
          }
	}
      }
      }




    free(fasta);
    //    printf("Slave rank %d: Sending data back to master...\n", myRank);
    MPI_Send(&recieved, 1, dataMPI, 0, 0, MPI_COMM_WORLD);
    //    printf("Slave rank %d: Data successfully sent back to master\n\n", myRank);
  }
}


char * readFasta(char * filename) {
  FILE * f = fopen(filename, "r");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char * out = malloc(fsize + 1);
  fread(out, fsize, 1, f);
  fclose(f);
  out[fsize] = 0;
  return out;
}
