#include "ct_threadpool.h"

CtThreadPool::CtThreadPool(int num,bool isCheck,int checkTimeVal)
{
  //this->thread_init_num = num;
  this->is_established = false;
  this->is_destroy     = false;
  this->is_check       = isCheck;
  this->check_timeval  = checkTimeVal;
  this->cb_check       = CtThreadPool::thread_pool_need_check;
  this->cb_display     = CtThreadPool::display_thread_info;
  this->cb_destroy     = CtThreadPool::thread_pool_need_destroy;
  this->cb_extend      = CtThreadPool::thread_pool_need_extend;
  this->cb_execute     = CtThreadPool::thread_execute_entry;
 
  int thread_number = this->get_cpu_core_sum() * 2;
  if(num > thread_number && num < THREAD_MAX_NUM) thread_number = num;
  else if(num > THREAD_MAX_NUM) thread_number = THREAD_MAX_NUM;  
 
  this->thread_pool_create(thread_number);  
}

CtThreadPool::~CtThreadPool()
{
  this->thread_pool_destroy();
}

int CtThreadPool::get_cpu_core_sum()
{
  int NUM_PROCS = sysconf(_SC_NPROCESSORS_ONLN);

  return NUM_PROCS;
}

int CtThreadPool::get_cpu_sum()
{
  
}

bool CtThreadPool::thread_pool_create(int num)
{
  if(num < 1) return is_established;

  this->thread_init_num    = num;
  this->thread_current_num = num;
  this->thread_run_num     = num;
  this->thread_killed_num  = 0;
  this->queue_size         = 0;

  workers_header = new thread_worker_t();
  threads_header = new thread_info_t();

  workers_indexer  = workers_header;
  threads_indexer = threads_header;

  pthread_mutex_init(&lock_worker_list,NULL);
  pthread_mutex_init(&lock_thread_list,NULL);
  pthread_cond_init(&cond_queue_ready,NULL);
  pthread_cond_init(&cond_need_destroy,NULL);
  pthread_cond_init(&cond_need_extend,NULL);

  //create thread pool
  thread_info_t *tmp = NULL;
  for(int i = 0; i < thread_init_num; i++)
  {
	  tmp = new thread_info_t();
	  if( NULL == tmp ) return is_established;
      
	  threads_indexer -> next = tmp; //tail insert
	  threads_indexer = tmp;

      int retval = pthread_create(&(tmp->id),NULL,this->cb_execute,this);
	  if( retval != 0) return is_established;

	  tmp->state = THREAD_STATE_RUN;

           sleep(1);
  }
  
  //create display thread
  int retval = pthread_create(&(this->display_thread),NULL,this->cb_display,this);
  if(retval != 0) return is_established;

  if(is_check)
  {
   //create dynamical destroy thread
   retval = pthread_create(&destroy_thread,NULL,this->cb_destroy,this);
   if(retval != 0) return is_established;

   //create dynamical extend thread
   retval = pthread_create(&extend_thread,NULL,this->cb_extend,this);
   if(retval != 0) return is_established;

   //create check
   retval = pthread_create(&check_thread,NULL,this->cb_check, this);
   if(retval != 0) return is_established;
  }
  this->is_established = true;
  return this->is_established; 
}

int CtThreadPool::thread_pool_add_worker(ThreadEntryFn fn, void *arg)
{
  if(!is_established) return -1;

  thread_worker_t *worker = new thread_worker_t();
  worker->cbTask = fn;   //Task callback
  worker->arg    = arg;
  worker->next   = NULL;
  
  pthread_mutex_lock(&lock_worker_list);
  workers_indexer -> next = worker; //tail insert
  workers_indexer = worker; 
  this->queue_size ++;
  pthread_mutex_unlock(&lock_worker_list);

  //sleep(2); //too slow after adding this sentence.
  pthread_cond_signal(&cond_queue_ready);
}

int CtThreadPool::thread_pool_wait()
{
  if(!is_established) return -1;
  
  thread_info_t *thread = this->threads_header;
  while(thread->next != NULL)
  {
	   thread->next->state = THREAD_STATE_EXIT;
	   pthread_join(thread->next->id,NULL);
	   thread = thread->next;
  }
  return 0;
}

void CtThreadPool::thread_pool_cancel()
{
  thread_info_t *thread = this->threads_header;
  while(thread->next != NULL)
  {
      pthread_cancel(thread->next->id);
      thread = thread->next;
  }
}

int CtThreadPool::thread_pool_destroy()
{
   if(!is_established) return -1;
   if(is_destroy)      return 0;
  
   //pthread_cond_broadcast(&cond_queue_ready); //Awake up the threads which blocked on Contidion cond_queue_ready
   //thread_pool_wait();
     thread_pool_cancel();
    
   
   //Delete all thead_pool_t
   thread_info_t *thread = this->threads_header->next;
   thread_info_t *tmp    = NULL;
   while(thread != NULL)
   {
	   tmp = thread->next;
	   delete thread;
       thread = tmp;
   }
   delete this->threads_header;

   //delete all worker tasks
   thread_worker_t *worker = this->workers_header->next;
   thread_worker_t *tmp1   = NULL;
   while(worker != NULL)
   {
     tmp1 = worker->next;
	 delete worker;
	 worker = tmp1;
   }
   delete this->workers_header;
  
   pthread_cancel(this->display_thread);
   pthread_cancel(this->destroy_thread);
   pthread_cancel(this->extend_thread);
   pthread_mutex_destroy(&lock_worker_list);
   pthread_mutex_destroy(&lock_thread_list);
   pthread_cond_destroy(&cond_queue_ready);
   pthread_cond_destroy(&cond_need_destroy);
   pthread_cond_destroy(&cond_need_extend);

   is_established = false;
   is_destroy     = true;

   return 0;
}

thread_info_t* CtThreadPool::get_thread_by_id(pthread_t id)
{
  thread_info_t *thread = this->threads_header->next;
  while(thread != NULL)
  {
    if(thread->id == id) return thread;
	thread = thread->next;
  }

  return NULL;
}

void* CtThreadPool::thread_execute_entry(void *arg)
{

   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);           //允许退出线程   
   //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,   NULL);  //设置立即取消  
   pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);          //Exited when meet cancel point
   CtThreadPool *tp = static_cast<CtThreadPool*>(arg);
   while(1)
   {
      pthread_cleanup_push( (void (*)(void*)) pthread_mutex_unlock, (void*) &(tp->lock_worker_list));
      pthread_mutex_lock(&(tp->lock_worker_list));

	 pthread_t id = pthread_self();

	 thread_info_t *thread = tp->get_thread_by_id(id);

     if(tp->is_destroy == true && thread !=NULL )
     {
		 pthread_mutex_unlock(&(tp->lock_worker_list));
		 thread->state = THREAD_STATE_EXIT;
		 tp->thread_killed_num ++;
		 tp->thread_run_num --;
		 pthread_exit(NULL);
	 }

	 if(thread) thread->state = THREAD_STATE_TASK_WAITTING;

	 while(tp->queue_size <= 0 && !tp->is_destroy)
		 pthread_cond_wait(&(tp->cond_queue_ready),&(tp->lock_worker_list) );

         if(tp->queue_size > 0 ) tp->queue_size --;
	 thread_worker_t *worker = tp->workers_header->next;
	 tp->workers_header->next = worker->next;
	 pthread_mutex_unlock(&(tp->lock_worker_list));

	 if(thread) thread->state = THREAD_STATE_TASK_PROCESSING;
	 //execute task
	 (*(worker->cbTask))(worker->arg);
	 if(thread) thread->state = THREAD_STATE_TASK_FINISHED;

         delete worker;
         pthread_cleanup_pop(0);
	 sleep(1);
   }
}

void* CtThreadPool::thread_pool_need_check(void *arg)
{
  CtThreadPool *tp = static_cast<CtThreadPool*>(arg);
  while(1)
  {
      sleep(tp->check_timeval);
      std::cout<<"Checking thread pool usage rate..."<<std::endl;
      if(tp->queue_size <= 0) 
         pthread_cond_signal(&(tp->cond_need_destroy));
     
      if(tp->queue_size > tp->thread_current_num * THREAD_POOL_BUSY_WEIGHT)
        pthread_cond_signal(&(tp->cond_need_extend));
  }
}

void* CtThreadPool::thread_pool_need_extend(void *arg)
{
  CtThreadPool *tp = static_cast<CtThreadPool*>(arg);
  while(1)
  {
    pthread_mutex_lock(&(tp->lock_thread_list));
    while(tp->queue_size > tp->thread_current_num * THREAD_POOL_BUSY_WEIGHT)
	pthread_cond_wait(&(tp->cond_need_extend),&(tp->lock_thread_list) );

    if(tp->thread_current_num < THREAD_MAX_NUM-1)
	{
        thread_info_t *tmp = new thread_info_t();
		if(tmp != NULL)
		{
			tp->threads_indexer ->next = tmp;
			tp->threads_indexer = tmp;

			tp->thread_current_num ++;
			tp->thread_run_num ++;
			pthread_create(&(tmp->id),NULL,tp->cb_execute,tp);
			tmp->state = THREAD_STATE_RUN;
		}
	}
    pthread_mutex_unlock(&(tp->lock_thread_list));
	sleep(1);
  }
}

void * CtThreadPool::thread_pool_need_destroy(void *arg)
{
   CtThreadPool *tp = static_cast<CtThreadPool*>(arg);
   while(1)
   {
    pthread_mutex_lock(&(tp->lock_thread_list));
    while(tp->queue_size <= 0)
      pthread_cond_wait(&(tp->cond_need_destroy),&(tp->lock_thread_list));
  
    thread_info_t *tmp  = tp->threads_header->next;
	thread_info_t *tmp1 = NULL;
    while(tmp != NULL)
    {
	   if(tmp->state != THREAD_STATE_TASK_PROCESSING && tp->thread_current_num > THREAD_MIN_NUM)
	   {
		   
          pthread_cancel(tmp->id);
          tmp1 = tmp->next;
		  delete tmp;
		  tmp = tmp1;

		  tp->thread_run_num --;
		  tp->thread_current_num --;
		  tp->thread_killed_num ++;
	   }
           else
             tmp = tmp->next;
    }
	pthread_mutex_unlock(&(tp->lock_thread_list));
	sleep(1);
   }
}

void * CtThreadPool::display_thread_info(void *arg)
{
  CtThreadPool *tp = static_cast<CtThreadPool *>(arg);
  while(1)
  {
   std::cout<<"Threads "<<tp->thread_current_num<<",Running "<<tp->thread_run_num
            <<",Killed "<<tp->thread_killed_num<<std::endl;
	
	sleep(50);
  }
}
