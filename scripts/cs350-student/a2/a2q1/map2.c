/* map.c
 * ----------------------------------------------------------
 *  CS350
 *  Assignment 1
 *  Question 1
 *
 *  Purpose:  Gain experience with threads and basic
 *  synchronization.
 *
 *  YOU MAY ADD WHATEVER YOU LIKE TO THIS FILE.
 *  YOU CANNOT CHANGE THE SIGNATURE OF MultithreadedWordCount.
 * ----------------------------------------------------------
 */
#include "data.h"

#include <stdlib.h>
#include <string.h>

/* --------------------------------------------------------------------
 * MultithreadedWordCount
 * --------------------------------------------------------------------
 * Takes a Library of articles containing words and a word.
 * Returns the total number of times that the word appears in the
 * Library.
 *
 * For example, "There the thesis sits on the theatre bench.", contains
 * 2 occurences of the word "the".
 * --------------------------------------------------------------------
 */

struct arg_struct{
  struct Library *lib;
  char *word;
  int start;
  int is_last;
  size_t count;
};


void *thread_function(void *argument){
  if (argument==NULL){
    perror("NULL ARGUMENT");
  }
  struct arg_struct *args = (struct arg_struct *)argument;
  printf("THREAD index: %d\n", args->start);
  
  int stop = -1;
  if (args->is_last == 0){
    // not last
    stop = args->start + args->lib->numArticles;
  }
  else{
    stop = args->lib->numArticles / NUMTHREADS;
  }

  size_t local_count = 0;
  printf("here\n");
  for (unsigned int i = args->start; i < stop; i ++){
    if (args->is_last == 1) { printf("last thread i:%d", i); }

    struct Article * art = args->lib->articles[i];
    for ( unsigned int j = 0; j < art->numWords; j++){
      // Get the length of the function.
      size_t len = strnlen( art->words[j], MAXWORDSIZE );
      if ( !strncmp( art->words[j], args->word, len)){
        local_count += 1;
      }
    }
  }

  args->count = local_count;
  return args;
}

size_t MultithreadedWordCount( struct  Library * lib, char * word)
{
  printf("Parallelizing with %d threads...\n",NUMTHREADS);

  pthread_t thread_ids[NUMTHREADS];
  printf("NUMTHREADS:%d\n", NUMTHREADS);

  for (int i=0; i<NUMTHREADS; i++){
    struct arg_struct *args;
    args = malloc(sizeof(struct arg_struct));
    args->lib = lib;
    args->word = word;
    args->start = (lib->numArticles / NUMTHREADS) * i;
    if (i < NUMTHREADS-1){
      // not last thread
      args->is_last = 0;
    }
    else { args->is_last = 1; }

    printf("main i: %d\n", i);
    printf("main args.index: %d\n", args->start);

    if (pthread_create(&thread_ids[i], NULL, thread_function, (void *)args) != 0){
      perror("failed pthread_create");
      return -1;
    }
    printf("\n");
  }

  size_t total_count = 0;
  struct arg_struct *args;
  for (int i=0; i<NUMTHREADS; i++){
    if (pthread_join(thread_ids[i], (void **) &args) != 0){
      perror("failed pthread_join");
    }
    total_count += args->count;
  }
  free(args);

  printf("Final counter value: %lu\n", total_count);
  return total_count;
}


// can try global var
// or commit https://stackoverflow.com/questions/38704939/pass-argument-to-multiple-threads 
// final fix: https://stackoverflow.com/questions/36526259/split-c-array-into-n-equal-parts