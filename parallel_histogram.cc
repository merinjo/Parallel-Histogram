#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include "carbon_user.h"
#include <pthread.h>
#include <algorithm>

using namespace std;

void* threadMain(void* arg);
int num_threads = 64;
pthread_barrier_t barrier; 
int nKeys = 512*1024;             // Total number of keys
unsigned int MaxKey = 2147483648; // Allowable keys in the range 0 ... MaxKey-1
int *Keys;
int *bin;	
int size = 0;
int bsize=0;	
int **counts;

int main(int argc, char* argv[]) {
   
   printf("# keys: %i MaxKey: %u\n", nKeys, MaxKey);

   int rv;
   FILE *file;

   if (argc != 2) {
      printf("Usage:\n\t%s <input_key>\n", argv[0]);
      exit(1);
   }
   
   // Generate the Keys
   Keys = (int*) malloc(sizeof(int)*nKeys);

   char *inputFile = argv[1];
   file = fopen(inputFile, "r");
   if (file == NULL) {
      printf("ERROR: Unable to open file '%s'.\n", inputFile);
      exit(1);
   }
   //Reading keys from file 
   for (int j=0; j< nKeys; j+=10) {
      rv = fscanf(file, "%u %u %u %u %u %u %u %u %u %u", &Keys[j], &Keys[j+1], &Keys[j+2], &Keys[j+3], &Keys[j+4], &Keys[j+5], &Keys[j+6], &Keys[j+7], &Keys[j+8], &Keys[j+9]);
      if (rv != 10) {
         printf("ERROR: Unable to read from file '%s'.\n", inputFile);
         fclose(file);
         exit(1);
      }
   }
   fclose(file);
   
   // Unique Keys goes to Bins
    size = 1;
	int flag;
	int *bins = (int*) malloc(sizeof(int)*nKeys);
	bins[0] = Keys[0];
	for(int i=1; i<nKeys; i++)
	{
		flag = 0;
		for(int j = 0; j < size; j++) 
		{
			if(bins[j]==Keys[i])
			{	
				flag = 1;
				break;
			}
		}
		if(flag == 0)
		{
			bins[size]=Keys[i];
			size++;
		}
		
	}

	if(size % num_threads != 0)
	{
		bsize = (size/num_threads + 1)*num_threads;
	}
	printf("Size of bin  %i\n", bsize);
	
	bin = (int*) malloc(sizeof(int)*bsize);
	for(int j = 0; j < size; j++)
		bin[j] = bins[j];
	free(bins);

   // Allocate and initialize storage for counters for Privatization  
   counts = (int**) malloc(num_threads*sizeof(int*));
   for(int j = 0; j < num_threads; j++)
	{
		counts[j] = (int*) malloc(bsize*sizeof(int));
			for(int i = 0; i < bsize; i++)
				counts[j][i] = 0;
	}

   int thread_args[num_threads];
   for(int i = 0; i < num_threads; i++)
   {
      thread_args[i] = i;
   }
   
   pthread_barrier_init(&barrier, NULL, num_threads);	

   CarbonEnableModels();
   
   pthread_t thread_handles[num_threads];
   for (int i = 1; i < num_threads; i++)
   {
       int ret = pthread_create(&thread_handles[i], NULL, threadMain, (void*) &thread_args[i]);
       if (ret != 0)
       {
           fprintf(stderr, "ERROR spawning thread %i\n", i);
           exit(EXIT_FAILURE);
       }
                              }
    threadMain((void*) &thread_args[0]);

    #ifdef DEBUG
      fprintf(stderr, "Created Threads.\n");
    #endif

    for (int i = 1; i < num_threads; i++)
    {
       pthread_join(thread_handles[i], NULL);
    }
    
	CarbonDisableModels();

   // checker: print counts for a range of keys
   int LOKey = 10;
	int HIKey = 20;
	for (int i=LOKey; i<HIKey; i++)
		for (int j=0; j<size; j++)
			if(bin[j]==Keys[i])
			{
				printf("Key countser at Keys[%d] = %d\n", i, counts[0][j]);
				break;
			}


   free(Keys);
   free(counts);
   return 0;
}

void* threadMain(void* arg)
{
	
	int* args = (int*) arg;
    int tid = *args;
	
	int segment_size = nKeys/num_threads; 
	int output_segment_size = bsize/num_threads;
	int start = tid*output_segment_size;
	int end = (tid+1)*output_segment_size;	
	
	for(int i = 0; i < segment_size; i++)
	{
		for(int k = i*num_threads; k < (i+1)*num_threads; k++)
		{
			if(k % tid == 0)
			{
				for(int j=0; j<size; j++)
					if(Keys[k] == bin[j])
					{
						counts[tid][j]++;
						break;
					}	 				
			}
			
		}
		
	}
	
	pthread_barrier_wait(&barrier);
	
		//Add the results
	for(int i = start; i < end; i++)
		for(int j=1; j<num_threads; j++)
			counts[0][i] += counts[j][i];
}












