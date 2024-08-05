/**
 * A micro-benchmark exhibit WR /RW true sharing  
 * Thread 0 perform writes to variable ts_var
 * Thread 1,2,3 perform read of ts_var and assign to local variable x
 */

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef ROI_TRACING
  #include "m5_library/hooks_prospar.h"
#endif

#define NUM_THREADS 4

int loop_count __attribute__((aligned(64))) = 0;
void* only_ts(void*);

uint32_t ts_var __attribute__((aligned(64))) = 0;

pthread_mutex_t lock_var __attribute__((aligned(64)));

void* only_ts(void* threadId) {
  int currID = (intptr_t)threadId;
  int x = 0;
  for (uint32_t i = 0; i < loop_count; i++) {
    // WR RD true sharing.
    if (currID == 0) { //write by thread 0
      pthread_mutex_lock(&lock_var);
      ts_var += 1;
      pthread_mutex_unlock(&lock_var);
    } else { // all other thread should perform read.
      x = ts_var;
      x--; // to discourage unused variable error.
    }
  }
  return NULL;
}

int main(int argc, char* argv[]) {
  if (argc == 2) {
    loop_count = (1 << atoi(argv[1]));
  } else {
    loop_count = (1 << 18);
  }

  pthread_t threads[NUM_THREADS];

  printf("Starting address of TS_SharedVar :%p \n", (void*)&ts_var);
  printf("Address of loop count var: %p \n", (void*)&loop_count);
  printf("Address of lock var: %p \n", (void*)&lock_var);
  #ifdef ROI_TRACING
    roi_begin();
  #endif
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(&threads[i], NULL, &only_ts, (void*)(intptr_t)i);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  #ifdef ROI_TRACING
    roi_end();
  #endif
  if (ts_var != loop_count) {
    printf("Diff found for shared var: %d", ts_var);
  }
  return EXIT_SUCCESS;
}
