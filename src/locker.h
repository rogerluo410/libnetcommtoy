#ifndef LOCKER_H
#define LOCKER_H

#include<pthread.h>
#include<semaphore.h>
#include<assert.h>

class Sem
{
public:
   Sem()
   { 
	 assert(sem_init(&m_sem,0,0) == 0 );
   }
   ~Sem()
   {
	   sem_destroy(&m_sem);
   }
   bool wait()
   {
	   return sem_wait(&m_sem) == 0;
   }
   bool post()
   {
	   return sem_post(&m_sem) == 0;
   }
private:
   sem_t m_sem;
};

class Locker
{
public:
    Locker()
	{
       assert( pthread_mutex_init(&m_mutex, NULL) == 0 );
	}
	~Locker()
	{
		pthread_mutex_destroy(&m_mutex);
	}
	bool lock()
	{
		return pthread_mutex_lock(&m_mutex) == 0;
	}
	bool unlock()
	{
		return pthread_mutex_unlock(&m_mutex) == 0;
	}
private:
    pthread_mutex_t m_mutex;
};

class MutexGuard
{
public:
	MutexGuard(Locker& mutex)
	{
		m_mutex = mutex;
		m_mutex.lock();
	}
	~MutexGuard()
	{
		m_mutex.unlock();
	}
private:
	Locker m_mutex;
};

class Cond
{
public:
   Cond()
   {
	   assert(pthread_mutex_init(&m_mutex,NULL) == 0 );
	   assert(pthread_cond_init(&m_cond,NULL) == 0 );
   }
   ~Cond()
   {
	   pthread_mutex_destroy(&m_mutex);
	   pthread_cond_destroy(&m_cond);
   }
   bool wait()
   {
     int ret = 0;
	 pthread_mutex_lock(&m_mutex);
	 ret = pthread_cond_wait(&m_cond, &m_mutex);
	 pthread_mutex_unlock(&m_mutex);
	 return ret == 0;
   }
   bool signal()
   {
	   return pthread_cond_signal(&m_cond) == 0;
   }
private:
 	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
};


#endif
