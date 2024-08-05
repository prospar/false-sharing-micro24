// FalseSharing: default input size reduced by 1024 times
// input-size: 1 << input
// total access: 3 * input size
// each thread different field of different structure .
// access to structure of different field of same structure separated by barrier.

// Insignificant amount of false sharing
#include <cassert>
#include <iostream>
#include <pthread.h>
#include <thread>
#ifdef ROI_TRACING
#include "hooks_prospar.h"
#endif

#define CACHE_LINE_SIZE (64)
#define NUM_THREADS (2)
using namespace std;

struct alignas(CACHE_LINE_SIZE) {
  int x;
  int y;
} A;
struct alignas(CACHE_LINE_SIZE) {
  int x;
  int y;
} B;
struct alignas(CACHE_LINE_SIZE) {
  int x;
  int y;
} C;

long N;

pthread_barrier_t barrier;

// Accesses are separated by a barrier hence although there is false sharing
// it should be insignificant compared to the total number of samples taken in execution.

void Work(int me) {
  for (long i = 0; i < N; i++) {
    if ((me & 1) == 0) {
      A.x = 1;
    } else {
      B.y = 1;
    }
  }
  pthread_barrier_wait(&barrier);
  for (long i = 0; i < N; i++) {
    if ((me & 1) == 0) {
      B.x = 1;
    } else {
      C.y = 1;
    }
  }
  pthread_barrier_wait(&barrier);
  for (long i = 0; i < N; i++) {
    if ((me & 1) == 0) {
      C.x = 1;
    } else {
      A.y = 1;
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc == 2) {
    N = (1 << atoi(argv[1]));
  } else {
    N = (1 << 18); // FalseSharing: original N 1<< 28
  }
  pthread_barrier_init(&barrier, 0, NUM_THREADS);
#ifdef ROI_TRACING
  roi_begin();
#endif
  thread threadObj1([] { Work(0); });
  thread threadObj2([] { Work(1); });
  threadObj1.join();
  threadObj2.join();
  pthread_barrier_destroy(&barrier);
#ifdef ROI_TRACING
  roi_end();
#endif
  cout << "Test might detect false sharing among pair A.x:A.y, B.x:B.y, and C.x:C.y, but the "
          "amount of false sharing must be insignificant compared to the number of samples taken\n";
  return 0;
}
