// FalseSharing: default input size reduced 1024 times

// Basic R-W false sharing test

#include <cassert>
#include <iostream>
#include <thread>
#ifdef ROI_TRACING
#include "hooks_prospar.h"
#endif

#define CACHE_LINE_SIZE (64)
using namespace std;

struct alignas(CACHE_LINE_SIZE) {
  int x;
  int y;
} A;

long N;

/* We expect samples to be reporting R-W or W-R false sharing */

void Work(int me) {
  for (long i = 0; i < N; i++) {
    if ((me & 1) == 0)
      A.x += 1; // MUST be involved in R-W or W-R false sharing
    else
      A.y += 1; // MUST be involved in R-W or W-R false sharing
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
  assert(A.x == N);
  assert(A.y == N);
  cout << "Test should find its samples involved in R-W or R-W false sharing\n";
  return 0;
}
