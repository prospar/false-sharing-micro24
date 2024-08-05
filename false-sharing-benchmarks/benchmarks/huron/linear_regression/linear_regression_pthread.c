/* Copyright (c) 2007, Stanford University
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Stanford University nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "stddefines.h"

#ifdef ROI_TRACING
#include "hooks_prospar.h"
#endif
#ifdef SE_MODE_BUILD
#include "gem5/m5ops.h"
#endif
typedef struct {
  char x;
  char y;
} POINT_T;

typedef struct {
  pthread_t tid;
  POINT_T* points;
  int num_elems;
  long long SX;
  long long SY;
  long long SXX;
  long long SYY;
  long long SXY;
  //char padding[4];
} lreg_args;

/* linear_regression_pthread
 *
 */
void* linear_regression_pthread(void* args_in) {
  lreg_args* args = (lreg_args*)args_in;
  int i, j;

  args->SX = 0;
  args->SXX = 0;
  args->SY = 0;
  args->SYY = 0;
  args->SXY = 0;

  // char name[100];
  // sprintf(name, "th_%lu.txt", args->tid % 1000);
  // FILE *f = fopen(name, "w");
  // fprintf(f, "num_elems = %d\n\n", args->num_elems);
  // ADD UP RESULTS
  // FalseSharing: only execute single iteration for large input
  // fprintf("x, y = %d, %d\n", args->points[0].x, args->points[0].y);

  for (j = 0; j < 10; j++) {
    for (i = 0; i < args->num_elems; i++) {
        // fprintf(f,"%c %c  ",args->points[i].x, args->points[i].y);
        // fprintf(f, "x, y = %d, %d\n", args->points[i].x, args->points[i].y);
        // fprintf(f, "SX, SY, SYY, SXX, SXY = %lld, %lld, %lld, %lld, %lld\n",
        // args->SX, args->SY, args->SYY, args->SXX, args->SXY);
      //Compute SX, SY, SYY, SXX, SXY
      args->SX += args->points[i].x;
      args->SXX += args->points[i].x * args->points[i].x;
      args->SY += args->points[i].y;
      args->SYY += args->points[i].y * args->points[i].y;
      args->SXY += args->points[i].x * args->points[i].y;
    }
  }

  return (void*)0;
}

int main(int argc, char* argv[]) {
  int fd;
  char* fdata;
  char* fname;
  struct stat finfo;
  clock_t start_time, end_time;
  int req_units, num_threads, num_procs = 4, i;
#ifdef THREADS
  num_procs = THREADS;
#endif
  pthread_attr_t attr;
  lreg_args* tid_args;

  // Make sure a filename is specified
  if (argv[1] == NULL) {
    printf("USAGE: %s <filename>\n", argv[0]);
    exit(1);
  }
  printf("size of point_t struct %ld\n",sizeof(POINT_T));
  fname = argv[1];
  //printf("%d\n",sizeof(pthread_t));

  // Read in the file
  CHECK_ERROR((fd = open(fname, O_RDONLY)) < 0);
  // Get the file info (for file length)
  CHECK_ERROR(fstat(fd, &finfo) < 0);
  // Memory map the file
  CHECK_ERROR((fdata = mmap(0, finfo.st_size + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) ==
              NULL);

  //CHECK_ERROR((num_procs = sysconf(_SC_NPROCESSORS_ONLN)) <= 0);
  printf("The number of processors is %d\n\n", num_procs);

  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  num_threads = num_procs;

  printf("Linear Regression P-Threads: Running...\n");

  POINT_T* points = (POINT_T*)fdata;
  // FalseSharing: n tracks total elements
  long long n = (long long)finfo.st_size / sizeof(POINT_T);
  // FalseSharing: req_units tracks elements per thread
  req_units = n / num_threads;
  tid_args = (lreg_args*)malloc(sizeof(lreg_args) * num_procs);
  memset(tid_args, 0, sizeof(lreg_args) * num_procs);

  start_time = clock();

#ifdef ROI_TRACING
  roi_begin();
#endif
#ifdef SE_MODE_BUILD
  m5_reset_stats(0, 0);
#endif
  // Assign a portion of the points for each thread
  pthread_t tmps[64];
  for (i = 0; i < num_threads; i++) {
    tid_args[i].points = &points[i * req_units];
    int tmp_req_units = req_units;
    if (i == (num_threads - 1))
      tmp_req_units = n - i * req_units;
    tid_args[i].num_elems = tmp_req_units;
    tmps[i] = tid_args[i].tid;

    CHECK_ERROR(pthread_create(&tmps[i], &attr, &linear_regression_pthread, (void*)&tid_args[i]) !=
                0);
    tid_args[i].tid = tmps[i];
  }

  long long SX_ll = 0, SY_ll = 0, SXX_ll = 0, SYY_ll = 0, SXY_ll = 0;

  /* Barrier, wait for all threads to finish */
  for (i = 0; i < num_threads; i++) {
    long long ret_val;
    CHECK_ERROR(pthread_join(tid_args[i].tid, (void**)(void*)&ret_val) != 0);
    CHECK_ERROR(ret_val != 0);

    SX_ll += tid_args[i].SX;
    SY_ll += tid_args[i].SY;
    SXX_ll += tid_args[i].SXX;
    SYY_ll += tid_args[i].SYY;
    SXY_ll += tid_args[i].SXY;
  }
#ifdef ROI_TRACING
  roi_end();
#endif
#ifdef SE_MODE_BUILD
  m5_dump_stats(0, 0);
#endif
  end_time = clock();
  printf("Time to completion: %f seconds\n", (float)(end_time - start_time) / CLOCKS_PER_SEC);
  free(tid_args);

  double a, b, xbar, ybar, r2;
  double SX = (double)SX_ll;
  double SY = (double)SY_ll;
  double SXX = (double)SXX_ll;
  double SYY = (double)SYY_ll;
  double SXY = (double)SXY_ll;

  b = (double)(n * SXY - SX * SY) / (n * SXX - SX * SX);
  a = (SY_ll - b * SX_ll) / n;
  xbar = (double)SX_ll / n;
  ybar = (double)SY_ll / n;
  r2 = (double)(n * SXY - SX * SY) * (n * SXY - SX * SY) /
       ((n * SXX - SX * SX) * (n * SYY - SY * SY));

  printf("Linear Regression P-Threads Results:\n");
  printf("\ta    = %lf\n", a);
  printf("\tb    = %lf\n", b);
  printf("\txbar = %lf\n", xbar);
  printf("\tybar = %lf\n", ybar);
  printf("\tr2   = %lf\n", r2);
  printf("\tSX   = %lld\n", SX_ll);
  printf("\tSY   = %lld\n", SY_ll);
  printf("\tSXX  = %lld\n", SXX_ll);
  printf("\tSYY  = %lld\n", SYY_ll);
  printf("\tSXY  = %lld\n", SXY_ll);

  CHECK_ERROR(pthread_attr_destroy(&attr) < 0);
  CHECK_ERROR(munmap(fdata, finfo.st_size + 1) < 0);
  CHECK_ERROR(close(fd) < 0);
  return 0;
}