#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef ROI_TRACING
#include "hooks_prospar.h"
#endif
#ifdef SE_MODE_BUILD
  #include "gem5/m5ops.h"
#endif
#define THREAD_COUNT 4
#define TOTAL 1024
//#define ITER (1 << 11) // 1000000: original value
// FalseSharing: support for cmd param
int ITER = (1 << 12); // 1000000; original value

int* dynMemory; // FalseSharing: report by perf c2c
// FIXME: SB: I think you are trying to avoid false sharing on the lock variable. Why do you need
// 256 locks?
// VIPIN: Ideally each lock should be on separate cache line
pthread_mutex_t ownThreadLock[THREAD_COUNT * 64];
uint64_t align_factor = (64 / sizeof(int));
// printf("align_factor: %lu\n", align_factor);
//ptr is threadID
void* run(void* ptr) {
  int start = *((int*)ptr);
  printf("%d\n", start);
  for (int i = start; i < TOTAL; i += 4 * THREAD_COUNT) {
    for (int j = 0; j < ITER; j++) {
      pthread_mutex_lock(&ownThreadLock[start * 64]);
      int val = dynMemory[align_factor * i]; // FalseSharing: report by perf c2c 63%
      if (j == 0)
        val = 0;
      else {
        if (j % 2)
          val += 1;
        else
          val += 2;
      }
      dynMemory[align_factor * i] = val;                  // FalseSharing: report by perf c2c 8%
      val = dynMemory[align_factor * (i + THREAD_COUNT)]; // FalseSharing: report by perf c2c 9%
      if (j == 0)
        val = 0;
      else {
        if (j % 2)
          val += 1;
        else
          val += 2;
      }
      dynMemory[align_factor * (i + THREAD_COUNT)] = val;
      val = dynMemory[align_factor * (i + 2 * THREAD_COUNT)];
      if (j == 0)
        val = 0;
      else {
        if (j % 2)
          val += 1;
        else
          val += 2;
      }
      dynMemory[align_factor * (i + 2 * THREAD_COUNT)] = val;
      val = dynMemory[align_factor * (i + 3 * THREAD_COUNT)];
      if (j == 0)
        val = 0;
      else {
        if (j % 2)
          val += 1;
        else
          val += 2;
      }
      dynMemory[align_factor * (i + 3 * THREAD_COUNT)] = val;
      pthread_mutex_unlock(&ownThreadLock[start * 64]);
    }
  }
  pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
  if (argc > 1)
    ITER = (1 << atoi(argv[1]));

  // FIXME: SB: You should use a different version of malloc for aligned allocation (e.g.,
  // aligned_alloc() with C11 or posix_memalign()).
  uint64_t align_factor1 = 64 / sizeof(int);
  dynMemory = (int*)aligned_alloc(64, align_factor1 * TOTAL * sizeof(int));
  assert(((uintptr_t)dynMemory & 0x3F) == 0); // Ensure that the memory is 64-byte aligned
  printf("alignment %lu\n", align_factor);
  pthread_t threads[THREAD_COUNT];
  int params[THREAD_COUNT];
  for (int i = 0; i < THREAD_COUNT; i++) {
    params[i] = i;
    pthread_mutex_init(&ownThreadLock[i * 64], NULL);
  }

#ifdef ROI_TRACING
  roi_begin();
#endif
#ifdef SE_MODE_BUILD
  m5_reset_stats(0, 0);
#endif
  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_create(&threads[i], NULL, run, ((void*)(&params[i])));
  }
  for (int i = 0; i < THREAD_COUNT; i++) {
    pthread_join(threads[i], NULL);
  }

#ifdef ROI_TRACING
  roi_end();
#endif
#ifdef SE_MODE_BUILD
  m5_dump_stats(0, 0);
#endif

  return EXIT_SUCCESS;
}
