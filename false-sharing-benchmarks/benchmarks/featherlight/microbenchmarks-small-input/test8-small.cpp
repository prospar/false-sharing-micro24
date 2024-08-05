// FalseSharing: default input size reduced by 1024 times
// input size: 1 << input
// Proportional FS Total access: 6*inputsize A:3*input-size B: 2*input-size C:input-size

// Proportion of false sharing and permutation of source locations
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
struct alignas(CACHE_LINE_SIZE) {
  int x;
  int y;
} B;
struct alignas(CACHE_LINE_SIZE) {
  int x;
  int y;
} C;

long N;
// A test for proportion of false sharing and their locations
void Work(int me) {
  for (long i = 0; i < N; i++) {
    if ((me & 1) == 0) {
      A.x = 1; // Must contribute to 1/6 of total W-W false sharing
      A.x = 1; // Must contribute to 1/6 of total W-W false sharing
      A.x = 1; // Must contribute to 1/6 of total W-W false sharing
      B.x = 1; // Must contribute to 1/6 of total W-W false sharing
      B.x = 1; // Must contribute to 1/6 of total W-W false sharing
      C.x = 1; // Must contribute to 1/6 of total W-W false sharing
    } else {
      A.y = 1; // Must contribute to 1/6 of total W-W false sharing
      A.y = 1; // Must contribute to 1/6 of total W-W false sharing
      A.y = 1; // Must contribute to 1/6 of total W-W false sharing
      B.y = 1; // Must contribute to 1/6 of total W-W false sharing
      B.y = 1; // Must contribute to 1/6 of total W-W false sharing
      C.y = 1; // Must contribute to 1/6 of total W-W false sharing
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
  cout << "Test should find \n \
    3x3=9 pairs of contexts involved in W-W false sharing between the orderd pair A.x:A.y \n \
    3x3=9 pairs of contexts involved in W-W false sharing between the orderd pair A.y:A.x \n \
    2x2=4 pairs of contexts involved in false sharing between the ordered pair B.x:B.y \n \
    2x2=4 pairs of contexts involved in false sharing between the ordered pair B.y:B.x \n \
    1x1=1 context involved in W-W false sharing between the ordered pair C.x:C.y\n \
    1x1=1 context involved in W-W false sharing between the ordered pair C.y:C.x\n";
  return 0;
}
