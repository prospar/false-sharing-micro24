// FalseSharing: default input size by 1024 times
// input size: 1 << input
// TS on var A.x total access 1 << input by each thread

// True sharing NOT a false sharing test
#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#ifdef ROI_TRACING
#include "hooks_prospar.h"
#endif

#define CACHE_LINE_SIZE (64)
using namespace std;

struct alignas(CACHE_LINE_SIZE) {
  atomic<int> x;
} A;

long N;
/* There is contention (true sharing) but no false sharing */

void Work(int me) {
  for (long i = 0; i < N; i++) {
    A.x++; // Atomic Add: true sharing but not a false sharing
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
  assert(A.x == (2 * N));
  cout << "Test should find no false sharing at all\n";
  return 0;
}
