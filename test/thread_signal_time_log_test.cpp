#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include "signalmgr.h"
#include "log.h"

void sig_handler(int signo)
{
  std::cout<<"Signal " <<signo<<" came.."<<std::endl;
}

void *thread_entry(void * arg)
{
/*
  struct sigaction sa;
  memset(&sa,0,sizeof(sa));
  sa.sa_handler = sig_handler;
  sa.sa_flags |= SA_RESTART;
  sigfillset(&sa.sa_mask);
  sigaction(SIGINT,&sa,NULL);

  sigset_t thread_signal_mask;
  sigemptyset(&thread_signal_mask);

  sigaddset(&thread_signal_mask,SIGINT);
  pthread_sigmask(SIG_UNBLOCK,&thread_signal_mask,NULL);
  */
  while(1)
  {
    std::cout<<"In thread..."<<std::endl;
    LogAppend(FATAL,"test for thread %d",2);
	sleep(1);
  }
}




int main()
{
  char buf[32] = {0};
  time_t t;
  tm *tm_time = localtime(&t);
  snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
		   tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
		   tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);

  std::cout<<buf<<std::endl;

  struct timeval val;
  int sec = gettimeofday(&val,NULL);
  std::cout<<sec<<std::endl;

  Log::rootLog("./server_log.txt");

  CtSignalMgr sm;
  int sigset[5]={SIGINT,SIGTERM,SIGHUP,SIGSTOP,SIGPIPE};
  sm.add_set(sigset,5);
  pthread_t pid;
  pthread_t pid1;
  pthread_t pid2;
  pthread_t pid3;
  pthread_create(&pid,NULL,thread_entry,NULL );
  pthread_create(&pid1,NULL,thread_entry,NULL);
  pthread_create(&pid2,NULL,thread_entry,NULL);
  pthread_create(&pid3,NULL,thread_entry,NULL);
  pthread_join(pid,NULL);
  pthread_join(pid1,NULL);
  pthread_join(pid2,NULL);
  pthread_join(pid3,NULL);
  return 0;
}
