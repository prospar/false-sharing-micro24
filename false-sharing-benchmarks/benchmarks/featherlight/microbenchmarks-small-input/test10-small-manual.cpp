// Basic inter-process write-write false sharing test
#include <cassert>
#include <iostream>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <chrono>
#ifdef ROI_TRACING
#include "hooks_prospar.h"
#endif

#define CACHE_LINE_SIZE (64)
using namespace std;

struct alignas(CACHE_LINE_SIZE) A {
  int x;
  char pad[64];
  int y;
};

A* a;
long N;
/* We expects samples to be reporting inter-process W-W false sharing between the two processes */

void Work(int me) {
  for (long i = 0; i < N; i++) {
    if ((me & 1) == 0)
      a->x = 1; // W-W false share with A.y
    else
      a->y = 1; // W-W false share with A.x
  }
}

int main(int argc, char* argv[]) {
  a = (A*)mmap(0, sizeof(A), PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
  if (argc == 2) {
    N = (1 << atoi(argv[1]));
  } else {
    N = (1 << 18); // FalseSharing: original N 1<< 28
  }
#ifdef ROI_TRACING
  roi_begin();
#endif
  pid_t child = fork();
  if (child == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  if (child != 0) {
    Work(0);
    int wstatus;
    pid_t w = waitpid(child, &wstatus, 0);
    if (w == -1) {
      perror("waitpid");
      exit(EXIT_FAILURE);
    }
  } else {
    Work(1);
    return 0;
  }
#ifdef ROI_TRACING
  roi_end();
#endif
  cout << "Test should find samples to be involved in inter-process W-W false sharing\n";
  return 0;
}
