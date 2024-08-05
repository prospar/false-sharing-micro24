/**
 * A micro benchmark program to trigger sharing behavior
 * true and false sharing: Different cache line
 * Similar to initial access to falsely shared variable
 * later access to true sharing variable only
 * false and true sharing variable on different line
 */

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef ROI_TRACING
  #include "m5_library/hooks_prospar.h"
#endif

#define NUM_THREADS 4

int loop_count __attribute__((aligned(64)));
void* fs_ts_diffline(void*);

uint32_t array_with_fs[NUM_THREADS] __attribute__((aligned(64)));
uint32_t ts_var __attribute__((aligned(64))) = 0;

pthread_mutex_t lock_var __attribute__((aligned(64)));

int main(int argc, char* argv[]) {
  if (argc == 2) {
    loop_count = (1 << atoi(argv[1]));
  } else {
    loop_count = (1 << 21);
  }

  pthread_t threads[NUM_THREADS];

  printf("Starting address of array with FS : %p \n", (void*)&array_with_fs);
  printf("Starting address of TS_SharedVar :%p \n", (void*)&ts_var);
  printf("Address of loop_count variable:%p\n", (void*)&loop_count);
  printf("Address of lock variable:%p\n", (void*)&lock_var);

  uint64_t var_addr = (long unsigned int)&(array_with_fs);
  uint64_t arr_addr = (long unsigned int)&(ts_var);

  // Assert that shared_var and the start of shared_arr are not on the same line.
  if ((var_addr >> 6) == (arr_addr >> 6)) {
    printf("Variables allocated on same line\n");
  }

  #ifdef ROI_TRACING
    roi_begin();
  #endif
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(&threads[i], NULL, &fs_ts_diffline, (void*)(intptr_t)i);
  }

  // joining 4 threads i.e. waiting for all 4 threads to complete
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  #ifdef ROI_TRACING
    roi_end();
  #endif
  for (int i = 0; i < NUM_THREADS; i++) {
    if (array_with_fs[i] != loop_count)
      printf("Diff found for index: %d actual value:%d", i, array_with_fs[i]);
  }

  if (ts_var != loop_count)
    printf("Diff found for shared var: %d", ts_var);

  return 0;
}

void* fs_ts_diffline(void* threadId) {
  int currID = (intptr_t)threadId;
  int num_iteration = loop_count;
  array_with_fs[currID] = 0;
  for (uint32_t i = 0; i < num_iteration; i++)
    array_with_fs[currID] += 1;

  for (uint32_t i = 0; i < num_iteration; i++) {
    if (i % 4 == 0) {
      pthread_mutex_lock(&lock_var);
      ts_var += 1;
      pthread_mutex_unlock(&lock_var);
    }
  }
  return NULL;
}
