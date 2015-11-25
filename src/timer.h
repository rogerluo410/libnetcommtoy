#ifndef _TIMER_H
#define _TIMER_H

#include <time.h>
#include <netinet/in.h>
#include <pthread.h>
#include <iostream>

#define DEFAULT_SLOT_NUM 63
#define DEFAULT_WHEEL_INTERVAL 1

typedef void (*TimerCallBackFn)(void *);
typedef struct timer_worker_s{
   TimerCallBackFn cbTask;
   void *arg;
}timer_worker_t;

class CtTimer
{
public:
   CtTimer(int rot, int ts):next(NULL),prev(NULL),rotation(rot),time_slot(ts)
   {
     this->tt = new timer_worker_t();
   }
   ~CtTimer()
   {
	   if(this->tt != NULL)
	   {
		  delete this->tt;
		  this->tt = NULL;
	   }
   }
   void add_timer_task(TimerCallBackFn fn, void *arg)
   {
     this->tt->cbTask = fn;
	 this->tt->arg    = arg;
   }   
   timer_worker_t *get_timer_task()
   {
	    return this->tt;
   }
   int get_rotation()
   {
	    return this->rotation;
   }
   int get_time_slot()
   {
		return this->time_slot;
   }
public:
   int rotation;       //Make the timer be effective after specified rotation number.
   int time_slot;      //Set the slot id in timer wheel.
   timer_worker_t *tt; //Timer task
   CtTimer *next;      //Pointed to next timer
   CtTimer *prev;      //Pointed to previous timer
};

class CtTimeWheel      //Timer Wheel 
{
public:
	CtTimeWheel():cur_slot(0)
	{
		for(int i = 0; i < N; i++) slots[i] = NULL;
		pthread_mutex_init(&(this->lock_slots),NULL);
        pthread_create(&(this->tick_thread),NULL,tick_thread_entry,this);
		pthread_create(&(this->display_thread),NULL,display_thread_entry,this);
	}
	~CtTimeWheel()
	{
		//pthread_join(this->tick_thread,NULL);
		//pthread_join(this->display_thread,NULL);
		//pthread_cancel(this->tick_thread);
		//pthread_cancel(this->display_thread);
    
		pthread_join(this->tick_thread, NULL);
		pthread_join(this->display_thread,NULL);
		pthread_mutex_destroy(&(this->lock_slots));
		for(int i =0 ; i < N; i++)
		{
			CtTimer *tmp = slots[i];
			while(tmp)
			{
				slots[i] = tmp->next;
				delete tmp;
				tmp = slots[i];
			}
		}
	}
   
    //Display wheel information
	void display_wheel_info()
	{
      for(int i=0; i< N; i++)
	  {
           CtTimer *tmp = slots[i];
		   while(tmp)
		   {
			  std::cout<<"The rotation no is :"<<tmp->get_rotation()<<",the slot no is :"<<tmp->get_time_slot()<<std::endl;
			  tmp = tmp->next;
		   }
	  }
	}

	//Add timer to wheel
	int add_timer(int timeout,TimerCallBackFn fn,void *arg)
	{
      if(timeout < 0) return -1;
	  if(fn == NULL) return -1;

	  int ticks = 0;

	  if(timeout < SI) ticks = 1;
	  else ticks = timeout/SI;

	  int rotation   = ticks/N;
	  int ts         = (cur_slot + (ticks % N)) %N;  //slot index
	  CtTimer *timer = new CtTimer(rotation,ts);
	  timer->add_timer_task(fn,arg);

      pthread_mutex_lock(&(this->lock_slots));
	  if(!slots[ts])
		  slots[ts] = timer;
	  else
	  {
		  timer->next     = slots[ts];
		  slots[ts]->prev = timer;
		  slots[ts]       = timer;
	  }
	  pthread_mutex_unlock(&(this->lock_slots));
	  return 0;
	}

	//Delete timer
	/*
	void del_timer(CtTimer *timer)
	{
		if(!timer) return;

		int ts = timer->time_slot;
		if(timer == slots[ts])
		{
			slots[ts] = slots[ts]->next;
			if(slots[ts]) slots[ts]->prev = NULL;
            delete timer;
		}
		else
		{
			timer->prev->next = timer->next;
			if(timer->next) timer->next->prev = timer->prev;
			delete timer;
		}
	}*/

	//time tick
	void tick()
	{  
		CtTimer *tmp = slots[cur_slot];
		while(tmp)
		{
			if(tmp->rotation > 0)
			{
				tmp->rotation --;
				tmp = tmp->next;
			}
			else
			{
				tmp->get_timer_task()->cbTask(tmp->get_timer_task()->arg);
				if(tmp == slots[cur_slot])
				{
					slots[cur_slot] = tmp->next;
					delete tmp;
					if(slots[cur_slot]) slots[cur_slot]->prev = NULL;
					tmp = slots[cur_slot];
				}
				else
				{
					tmp->prev->next = tmp->next;
					if(tmp->next) tmp->next->prev = tmp->prev;
					CtTimer *tmp2 = tmp->next;
					delete tmp;
					tmp = tmp2;
				}
			}
		}
	   cur_slot = (cur_slot + 1) %N  ;	
	}

private:
	static void *tick_thread_entry(void *arg)
	{
	   std::cout<<"Tick thread start..."<<std::endl;
       CtTimeWheel *tw = static_cast<CtTimeWheel*>(arg);
	   while(1)
	   {
		 sleep(tw->SI);
		 pthread_mutex_lock(&(tw->lock_slots));
	     tw->tick();
		 pthread_mutex_unlock(&(tw->lock_slots));
	   }
	}
	static void *display_thread_entry(void *arg)
	{
	  std::cout<<"display thread start..."<<std::endl;
      CtTimeWheel *tw = static_cast<CtTimeWheel*>(arg);
	  while(1)
	  {
		  sleep(5);
          pthread_mutex_lock(&(tw->lock_slots)); 
          tw->display_wheel_info();
		  pthread_mutex_unlock(&(tw->lock_slots));
	  }
	}

private:
	static const int N  = DEFAULT_SLOT_NUM;
	static const int SI = DEFAULT_WHEEL_INTERVAL;
	CtTimer *slots[N];
	int cur_slot;
	pthread_t tick_thread;
	pthread_t display_thread;
	pthread_mutex_t lock_slots;
};
#endif
