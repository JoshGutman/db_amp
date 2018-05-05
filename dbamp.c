#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <mpi.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <omp.h>
#include <libgen.h>

#include "getfastas.h"
#include "search.h"
#include "extend.h"
#include "list.h"

#define MAX_PRIMER_LEN 100
#define MAX_NUM_AMPS 10
#define MAX_AMP_LEN 1001

int master(void);
void masterSend(char * fasta, char * forward, char * reverse, int * isAnti, int reciever, int kill);
int masterRecv(List * amps);

void slave(void);
char * readFasta(char * filename);


typedef struct Data {
  char    fasta[PATH_MAX];
  char    forward[MAX_PRIMER_LEN];
  char    reverse[MAX_PRIMER_LEN];
  int     kill;
  int     isAnti;
  int     numAmps;
} Data;

MPI_Datatype dataMPI;

int myRank;
int nprocs;

char * forward = NULL;
char * reverse = NULL;
char * input = NULL;
char * output = NULL;
int lower = 50;
int upper = 500;


int main(int argc, char * argv[]) {

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  
  // Create new MPI datatype
  int count = 6;
  int blockLengths[] = {PATH_MAX, MAX_PRIMER_LEN, MAX_PRIMER_LEN, 1, 1, 1};
  MPI_Aint disp[] = {offsetof(Data, fasta),
		     offsetof(Data, forward),
		     offsetof(Data, reverse),
		     offsetof(Data, kill),
		     offsetof(Data, isAnti),
		     offsetof(Data, numAmps)};
  MPI_Datatype types[] = {MPI_CHAR, MPI_CHAR, MPI_CHAR, MPI_INT, MPI_INT, MPI_INT};
  MPI_Type_create_struct(count, blockLengths, disp, types, &dataMPI);
  MPI_Type_commit(&dataMPI);
  

  // Read command-line flags
  int c;

  while((c = getopt(argc, argv, "f:r:i:o:l:u:")) != -1) {
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
    case 'l' :
      lower = atoi(optarg);
      break;
    case 'u' :
      upper = atoi(optarg);
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

  if (input && output && forward && reverse) {
    if (myRank == 0) {
      master();
    } else {
      slave();
    }
  } else {  // One of the required args wasn't passed in
    if (myRank == 0) {
      fprintf(stderr, "Incorrect usage.\n");
    }
  }


  MPI_Finalize();
  return 0;
}



int master(void) {

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

  //MPI_Status status;
  //Data recieved;
  int kill = 0;
  int sendTo;
  List amps;
  listInit(&amps, 1, MEDIUM, STRING);

  while (i < fastas.length*2) {

    //    printf("Master: Waiting for response...\n");
    //MPI_Recv(&recieved, 1, dataMPI, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    sendTo = masterRecv(&amps);

    //    printf("Master: Response recieved from %d\n", status.MPI_SOURCE);
    if (isAnti) {
      masterSend((char *)listGet(&fastas, i/2), antiForward, antiReverse, &isAnti, sendTo, kill);
    } else {
      masterSend((char *)listGet(&fastas, i/2), forward, reverse, &isAnti, sendTo, kill);
    }

    i++;
  }


  for (i = 1; i < nprocs; i++) {
    //MPI_Recv(&recieved, 1, dataMPI, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    masterRecv(&amps);
  }


  for (i = 1; i < nprocs; i++) {
    masterSend("", "", "", &isAnti, i, 1);
  }


  FILE * f = fopen(output, "w");
  for (i = 0; i < amps.length; i++) {
    fputs((char *)listGet(&amps, i), f);
  }

  fclose(f);



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
  toSend->numAmps = 0;
  //  printf("Master: Sending data to slave rank %d...\n", reciever);
  MPI_Send(toSend, 1, dataMPI, reciever, 0, MPI_COMM_WORLD);
  //  printf("Master: Data successfully sent to slave rank %d\n\n", reciever);
  *isAnti = !(*isAnti);
}


int masterRecv(List * amps) {
  MPI_Status status;
  Data recieved;
  int i;
  char amp[MAX_AMP_LEN];
  MPI_Recv(&recieved, 1, dataMPI, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  char * fastaBasename = basename(recieved.fasta);
  for (i = 0; i < recieved.numAmps; i++) {
    MPI_Recv(&amp, MAX_AMP_LEN, MPI_CHAR, status.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    char * toWrite = malloc( (((MAX_AMP_LEN/10)+1) + strlen(amp) + strlen(fastaBasename) + 3) * sizeof(char) );
    sprintf(toWrite, "%d\t%s\t%s\n%c", strlen(amp), amp, fastaBasename, '\0');
    listAppend(amps, (void *)toWrite);
  }
  return status.MPI_SOURCE;
}


void slave(void) {
  while (1) {
    Data recieved;
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
    List matches;
    #pragma omp parallel shared(fasta,forwardMatches,reverseMatches,forwards,reverses) private(i,j,matches)
    {
      #pragma omp for nowait
      for (i = 0; i < forwards.length; i++) {
	matches = search(fasta, (char *)listGet(&forwards, i));
	#pragma omp critical
	{
	  for (j = 0; j < matches.length; j++) {
	    listAppend(&forwardMatches, listGet(&matches, j));
	  }
	} // end pragma omp critical
      }

      #pragma omp for
      for (i = 0; i < reverses.length; i++) {
	matches = search(fasta, (char *)listGet(&reverses, i));
        #pragma omp critical
	{
          for (j = 0; j < matches.length; j++) {
            listAppend(&reverseMatches, listGet(&matches, j));
          }
	} // end pragma omp critical
      }

      #pragma omp barrier

    } // end pragma omp parallel


    // Look for matches within range
    int fwrd;
    int rvse;
    List amps;
    listInit(&amps, 1, SMALL, STRING);

    for (i = 0; i < forwardMatches.length; i++) {
      fwrd = *(int *)listGet(&forwardMatches, i);
      for (j = 0; j < reverseMatches.length; j++) {
	rvse = *(int *)listGet(&reverseMatches, i);
	if ( (fwrd < rvse) && (rvse - fwrd <= upper) && (rvse - fwrd >= lower) ) {
	  //strncpy(recieved.amps[numAmps++], fasta + fwrd + strlen(recieved.forward), rvse - fwrd);
	  char * amp = malloc(rvse-fwrd * sizeof(char));
	  strncpy(amp, fasta + fwrd + strlen(recieved.forward), rvse - fwrd);
	  amp[rvse-fwrd] = 0;
	  listAppend(&amps, (void *)amp);
	}
      }
    }
    recieved.numAmps = amps.length;

    free(fasta);
    //    printf("Slave rank %d: Sending data back to master...\n", myRank);
    MPI_Send(&recieved, 1, dataMPI, 0, 0, MPI_COMM_WORLD);
    for (i = 0; i < amps.length; i++) {
      MPI_Send((char *)listGet(&amps, i), MAX_AMP_LEN, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }
    //    printf("Slave rank %d: Data successfully sent back to master\n\n", myRank);
  }
}


char * readFasta(char * filename) {
  FILE * f = fopen(filename, "r");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char * tmp = malloc(fsize + 1);
  fread(tmp, fsize, 1, f);
  fclose(f);
  tmp[fsize] = 0;

  // Remove newlines
  char * out = malloc(fsize * sizeof(char));
  int i;
  int j = 0;
  for (i = 0; i < fsize; i++) {
    if (tmp[i] != '\n') {
      out[j++] = tmp[i];
    }
  }
  free(tmp);
  return out;
}
