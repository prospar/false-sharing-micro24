/**
 * A micro benchmark program to trigger different FS behavior
 * false sharing and true sharing on same cache line
 * alternate access to false and true sharing variable
 */
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#ifdef ROI_TRACING
  #include "m5_library/hooks_prospar.h"
#endif

#define NUM_THREADS 4

int loop_count __attribute__((aligned(64)));

struct FSTS {
  uint32_t shared_var;
  uint32_t shared_arr[NUM_THREADS];
};

struct FSTS fs_ts_struct __attribute__((aligned(64)));

pthread_mutex_t lock_var;

void* ts_fs(void* threadId) {
  int currID = (intptr_t)threadId;
  for (uint32_t i = 0; i < loop_count; i++) {
    pthread_mutex_lock(&lock_var);
    fs_ts_struct.shared_var += 1;
    pthread_mutex_unlock(&lock_var);
    fs_ts_struct.shared_arr[currID] += 1;
  }
  return NULL;
}

int main(int argc, char* argv[]) {
  if (argc == 2) {
    loop_count = (1 << atoi(argv[1]));
  } else {
    loop_count = (1 << 21);
  }
  pthread_t threads[NUM_THREADS];

  printf("Starting address of array with FS : %p \n", (void*)&fs_ts_struct.shared_arr);
  printf("Starting address of TS_SharedVar :%p \n", (void*)&fs_ts_struct.shared_var);
  printf("Address of loop_count variable:%p\n", (void*)&loop_count);
  printf("Address of lock variable:%p\n", (void*)&lock_var);

  // Assert that shared_var and the start of shared_arr are on the same line.
  uint64_t var_addr = (long unsigned int)&(fs_ts_struct.shared_var);
  uint64_t arr_addr = (long unsigned int)&(fs_ts_struct.shared_arr);

  // Assert that shared_var and the start of shared_arr are on the same line.
  if ((var_addr >> 6) != (arr_addr >> 6)) {
    printf("Variables not allocated on same line\n");
  }

  fs_ts_struct.shared_var = 0;
  for (int j = 0; j < NUM_THREADS; j++) {
    fs_ts_struct.shared_arr[j] = 0;
  }

  #ifdef ROI_TRACING
    roi_begin();
  #endif
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(&threads[i], NULL, &ts_fs, (void*)(intptr_t)i);
  }

  // joining 4 threads i.e. waiting for all 4 threads to complete
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  #ifdef ROI_TRACING
    roi_end();
  #endif
  // Correctness check
  for (int i = 0; i < NUM_THREADS; i++) {
    if (fs_ts_struct.shared_arr[i] != loop_count)
      printf("Diff found for index: %d actual value:%d\n", i, fs_ts_struct.shared_arr[i]);
  }

  if (fs_ts_struct.shared_var != 4 * loop_count) {
    printf("Diff found for shared var: %d\n", fs_ts_struct.shared_var);
  }

  return 0;
}
