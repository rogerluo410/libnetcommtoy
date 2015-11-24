#ifndef _CT_THREADPOOL_
#define _CT_THREADPOOL_
#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define THREAD_STATE_RUN             0
#define THREAD_STATE_TASK_WAITTING   1
#define THREAD_STATE_TASK_PROCESSING 2
#define THREAD_STATE_TASK_FINISHED   3
#define THREAD_STATE_EXIT            4

#define THREAD_MAX_NUM 20
#define THREAD_MIN_NUM 4
#define CHECK_TIMEVAL 100
#define THREAD_POOL_BUSY_WEIGHT 1.5

typedef void *(*ThreadEntryFn)(void *);
typedef struct thread_worker_s {
	ThreadEntryFn cbTask;
	void *arg;
	struct thread_worker_s *next;
} thread_worker_t;
typedef struct thread_info_s {
	pthread_t id;
	int state;
	struct thread_info_s *next;
}thread_info_t;

/*static char *thread_state_map[] = { 
	"Create",
	"Waitting",
	"Processing",
	"Finished",
	"Exit"};*/

class CtThreadPool
{
public:
  CtThreadPool( int num = THREAD_MIN_NUM , bool isCheck = true, int checkTimeVal = CHECK_TIMEVAL);
  ~CtThreadPool();
  
  bool thread_pool_create(int num);                        //Create pool
  int thread_pool_add_worker(ThreadEntryFn fn, void *arg); //Add task function
  int thread_pool_wait();                                  //Threads sync
  int thread_pool_destroy();                               //Destroy pool
  thread_info_t *get_thread_by_id(pthread_t id);           //Query thread  
  
private:
  int get_cpu_core_sum();
  int get_cpu_sum();
  void thread_pool_cancel();
  static void *display_thread_info(void *arg);
  static void *thread_execute_entry(void *arg);
  static void *thread_pool_need_extend(void *arg);
  static void *thread_pool_need_destroy(void *arg);
  static void *thread_pool_need_check(void *arg);

private:
  pthread_mutex_t lock_worker_list;
  pthread_mutex_t lock_thread_list;
  pthread_cond_t  cond_queue_ready;  //worker list ready
  pthread_cond_t  cond_need_destroy; 
  pthread_cond_t  cond_need_extend;
  thread_worker_t *workers_header;   //Task list header
  thread_worker_t *workers_indexer;   //Worker indexer which pointed to the end one
  thread_info_t   *threads_header;   //Thread list header
  thread_info_t   *threads_indexer;  //Threads indexer which pointed to the end one

  pthread_t display_thread;   //Print pool info thread
  pthread_t destroy_thread;   //Destroy thread
  pthread_t extend_thread;    //Extend thread
  pthread_t check_thread;     //Check thread

  ThreadEntryFn cb_display;   //Thread entry function for display
  ThreadEntryFn cb_destroy;   //Thread entry function for destroy while pool has idle threads 
  ThreadEntryFn cb_extend;    //Thread entry function for extend while tasks are more than threads 
  ThreadEntryFn cb_execute;   //Thread entry function for each processing thread in pool
  ThreadEntryFn cb_check;     //Thread entry function for checking
 
  bool is_destroy;            //Threadpool destroied flag
  bool is_established;        //Threadpool established flag
  bool is_check;              //Threadpool check flag
  int  check_timeval;         //Timeval for checking pool size
  int  thread_init_num;       //The number of initial threads
  int  thread_current_num;    //The number of threads
  int  thread_run_num;        //The number of running threads
  int  thread_killed_num;     //The number of killed threads
  int  queue_size;            //Queue size in the pool
};
#endif
