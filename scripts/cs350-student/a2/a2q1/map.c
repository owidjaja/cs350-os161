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

struct Library *global_lib;
char *global_word;
int chunk_size;

void *thread_handler(void *);

size_t MultithreadedWordCount( struct  Library * lib, char * word)
{
  printf("Parallelizing with %d threads...\n",NUMTHREADS);
    
    global_lib = lib;
    global_word = word;
    chunk_size = lib->numArticles / NUMTHREADS;

    pthread_t threads[NUMTHREADS];
    unsigned int i;

    for (i=0; i<NUMTHREADS; i++){
      int *index = malloc(sizeof(unsigned int));
      *index = i;
      if (pthread_create(&threads[i], NULL, &thread_handler, index) != 0) {
          perror("Failed to create thread");
      }
    }

    size_t total_sum = 0;
    for (i=0; i<NUMTHREADS; i++){
      int *ret;
      if (pthread_join(threads[i], (void **) &ret) != 0){
        perror("Failed to join thread");
      }
      total_sum += *ret;
      free(ret);
    }

    // printf("total_sum: %ld\n", total_sum);
    return total_sum;
}

void *thread_handler(void *arg){
  int thread_index = *((int *)arg);

    unsigned int start_index = (thread_index * chunk_size);
    unsigned int end_index = start_index + chunk_size;
    if (thread_index == NUMTHREADS - 1){
        end_index = global_lib->numArticles;
    }

    // printf("Index: %d\n", thread_index);
    // printf("Start Index: %d\n", start_index);
    // printf("End Index: %d\n", end_index);

    size_t local_count = 0;
    for(unsigned int i =  start_index; i < end_index;  i++)
    {
        // printf("Processing array element at: %d\n", i);

        struct Article * art = global_lib->articles[i];
        for ( unsigned int j = 0; j < art->numWords; j++){
          size_t len = strnlen( art->words[j], MAXWORDSIZE );
          if ( !strncmp( art->words[j], global_word, len)){
            local_count += 1;
          }
        }
    }

    *((int*)arg) = local_count;
    return arg;
}