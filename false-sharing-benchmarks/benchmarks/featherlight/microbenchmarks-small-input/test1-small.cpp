// FalseSharing: Reduce input size by 1024 times
// W-W false sharing instance
// Basic Write-Write false sharing test
#ifdef ROI_TRACING
#include "hooks_prospar.h"
#endif
#include <cassert>
#include <iostream>
#include <thread>
#define CACHE_LINE_SIZE (64)
using namespace std;

struct alignas(CACHE_LINE_SIZE) {
  int x;
  int y;
} A;

long N;
/* We expects samples to be reporting W-W false sharing */

void Work(int me) {
  for (long i = 0; i < N; i++) {
    if ((me & 1) == 0)
      A.x += 1; // W-W false share with A.y
    else
      A.y += 1; // W-W false share with A.x
  }
}

int main(int argc, char* argv[]) {
  if (argc == 2) {
    N = (1 << atoi(argv[1]));
  } else {
    N = (1 << 18); // FalseSharing: original N 1<< 28
  }
  cout << &(A.x) << " " << &(A.y) << "\n";
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
  cout << "Test should find samples to be involved in W-W false sharing\n";
  return 0;
}
