// FalseSharing: reduce problem size 1 << 14 times

// Basic no false sharing test
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

/* We expect no false sharing */

void Work(int me) {
  volatile int dummy = 0;
  for (long i = 0; i < N; i++) {
    if ((me & 1) == 0)
      dummy += A.x; // MUST NOT be any false sharing
    else
      dummy += A.y; // MUST NOT be any false sharing
  }
}

int main(int argc, char* argv[]) {
  if (argc == 2) {
    N = (1 << atoi(argv[1]));
  } else {
    N = (1 << 18); // FalseSharing: original N 1<< 32
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
  cout << "Test should find no false sharing at all\n";
  return 0;
}
