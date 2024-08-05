/**
 * A micro benchmark program that has no false sharing.
 *
 */
#define _GNU_SOURCE
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef ROI_TRACING
#include "m5_library/hooks_prospar.h"
#endif
#ifdef SE_MODE_BUILD
#include "gem5/m5ops.h"
#endif
#define NUM_THREADS 4

int loop_count __attribute__((aligned(64))) = 0;
void* without_fs(void*);

uint32_t array_without_fs[NUM_THREADS * 16]
    __attribute__((aligned(64))) = {0}; // per entry 4 bytes: 1 block per thread

// BEHAVIOR: NO FS NO TS (padded array)
void* without_fs(void* threadId) {
  int currID = (intptr_t)threadId;
  for (uint32_t i = 0; i < loop_count; i++) {
    array_without_fs[currID * 16] += 1;
  }
  printf("thread %d output %d\n", currID, array_without_fs[currID * 16]);
  return NULL;
}

int main(int argc, char* argv[]) {
  if (argc == 2) {
    loop_count = (1 << atoi(argv[1]));
  } else {
    loop_count = (1 << 21);
  }

  pthread_t threads[NUM_THREADS];
  clock_t start_time, end_time;

  printf("Starting address of array without FS : %p \n", (void*)&array_without_fs);
  printf("Address of loop_count var: %p\n", (void*)&loop_count);

  for (uint32_t i = 0; i < NUM_THREADS; i++) {
    array_without_fs[i] = 0;
  }

  start_time = clock();
#ifdef ROI_TRACING
  roi_begin();
#endif
#ifdef SE_MODE_BUILD
  m5_reset_stats(0, 0);
#endif
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(&threads[i], NULL, &without_fs, (void*)(intptr_t)i);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
#ifdef ROI_TRACING
  roi_end();
#endif
  end_time = clock();
  printf("Time to completion: %f seconds\n", (float)(end_time - start_time) / CLOCKS_PER_SEC);
#ifdef SE_MODE_BUILD
  m5_dump_stats(0, 0);
#endif
  // Correctness check
  for (int i = 0; i < NUM_THREADS; i++) {
    if (array_without_fs[i * 16] != loop_count)
      printf("Diff found for index: %d actual value:%d\n", i, array_without_fs[i]);
  }

  return 0;
}
