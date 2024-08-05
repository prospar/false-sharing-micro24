#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#ifdef ROI_TRACING
#include "hooks_prospar.h"
#endif
#ifdef SE_MODE_BUILD
   #include "gem5/m5ops.h"
#endif
#define THREAD_COUNT 4
#define TOTAL 1024*16
//#define ITER (1 << 11) // 1000000: original value
int ITER = (1 << 12); // 1000000: original value

int *dynMemory;
//pthread_mutex_t ownThreadLock[THREAD_COUNT*64];

void *run(void *ptr)
{
  int start = *((int *)ptr);
  printf("%d\n",start);
  for(int i = start; i < TOTAL; i+= 4*THREAD_COUNT)
  {
    for(int j = 0; j < ITER; j++)
    {
      //pthread_mutex_lock(&ownThreadLock[start*64]);
      int val=dynMemory[64*i];
      if(j == 0)val=0;
      else {
        if(j%2)val+=1;
        else val+=2;
      }
      dynMemory[64*i]=val;
      val=dynMemory[64*(i+THREAD_COUNT)];
      if(j == 0)val=0;
      else {
        if(j%2)val+=1;
        else val+=2;
      }
      dynMemory[64*(i+THREAD_COUNT)]=val;
      val=dynMemory[64*(i+2*THREAD_COUNT)];
      if(j == 0)val=0;
      else {
        if(j%2)val+=1;
        else val+=2;
      }
      dynMemory[64*(i+2*THREAD_COUNT)]=val;
      val=dynMemory[64*(i+3*THREAD_COUNT)];
      if(j == 0)val=0;
      else {
        if(j%2)val+=1;
        else val+=2;
      }
      dynMemory[64*(i+3*THREAD_COUNT)]=val;
      //pthread_mutex_unlock(&ownThreadLock[start*64]);
    }
  }
  pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
  if (argc > 1)
    ITER = (1 << atoi(argv[1]));
  dynMemory = (int *)malloc(64*TOTAL*sizeof(int));
  pthread_t threads[THREAD_COUNT];
  int params[THREAD_COUNT];
  for (int i = 0; i < THREAD_COUNT; i++)
  {
    params[i]=i;
    //pthread_mutex_init(&ownThreadLock[i*64], NULL);
  }
  #ifdef ROI_TRACING
    roi_begin();
  #endif
  #ifdef SE_MODE_BUILD
  m5_reset_stats(0, 0);
  #endif
  for (int i = 0; i < THREAD_COUNT; i++)
  {
    pthread_create(&threads[i], NULL, run, ((void *)(&params[i])));
  }
  for (int i = 0; i < THREAD_COUNT; i++)
  {
    pthread_join(threads[i], NULL);
  }
  #ifdef ROI_TRACING
    roi_end();
  #endif
  #ifdef SE_MODE_BUILD
  m5_dump_stats(0, 0);
  #endif
  return 0;
}

