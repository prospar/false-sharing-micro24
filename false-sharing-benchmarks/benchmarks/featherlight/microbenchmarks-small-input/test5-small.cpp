// FalseSharing: default input size reduced by 1024 times
// Num of iteration is cache_linesize/sizeof(int) times per the input as each access touch each byte of the block

// Array not involved in any false sharing
#include <cassert>
#include <iostream>
#include <thread>
#ifdef ROI_TRACING
#include "hooks_prospar.h"
#endif

#define CACHE_LINE_SIZE (64)
using namespace std;

struct alignas(CACHE_LINE_SIZE) {
  int x[CACHE_LINE_SIZE / sizeof(int)];
  int y[CACHE_LINE_SIZE / sizeof(int)];
} A;

long N;

// No false sharing at all.
void Work(int me) {
  for (long i = 0; i < N; i++) {
    if ((me & 1) == 0) {
      for (unsigned int j = 0; j < CACHE_LINE_SIZE / sizeof(int); j++)
        A.x[j] = 1; // MUST not be involved in false sharing
    } else {
      for (unsigned int j = 0; j < CACHE_LINE_SIZE / sizeof(int); j++)
        A.y[j] = 1; // MUST not be involved in false sharing
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc == 2) {
    N = (1 << atoi(argv[1]));
  } else {
    N = (1 << 18); // FalseSharing: original N 1<< 28
  }
#ifdef ROI_TRACING
  roi_begin();
#endif
  thread threadObj1([] { Work(0); });
  thread threadObj2([] { Work(1); });
  threadObj1.join();
  threadObj2.join();
#ifdef ROI_TRACING
  roi_end();
#endif
  cout << "Test should find no false sharing\n";
  return 0;
}
