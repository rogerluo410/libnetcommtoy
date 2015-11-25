#include "timer.h"


void time_task(void *arg)
{
 int *i = static_cast<int*>(arg);
 int j =*i;
  std::cout<<"It's time to execute the task["<<j <<"]."<<std::endl;
}

int main()
{
   
   CtTimeWheel *wheel = new CtTimeWheel();

   std::cout<<"start timer wheel..."<<std::endl;
   //create 10000 tasks
   for(int i =0 ; i < 10; i++)
   {
	  int timeout = 10;
	  if(i%2 == 0) timeout = 40;
      wheel->add_timer(timeout,time_task,&i);
	 // wheel->tick();
   }
	  
	 /* while(1)
		  {
	  wheel->tick();
	  sleep(1);
		  }*/
   //sleep(100);   
   //delete wheel;

    std::cout << "exe here.."<<std::endl;
   return 0;
}
