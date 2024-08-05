// FalseSharing: reduce input size by 1024 times

// Basic R-W /W-R false sharing test
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

/* We expect samples at A.x to be reporting R-W false sharing and samples at A.y to be reporting  W-R false sharing */

void Work(int me) {
  volatile int dummy = 0;
  for (long i = 0; i < N; i++) {
    if ((me & 1) == 0) {
      A.x = 1; // MUST be involved in  R-W false sharing
    } else {
      dummy += A.y; // MUST be involved W-R false sharing
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
  cout << "Test should find accesses to A.x at LBL1 to be involved in R-W false sharing and "
          "accesses to A.y at LBL2 to be involved in W-R false sharing\n";
  return 0;
}
