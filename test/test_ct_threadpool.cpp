#include <stdio.h>
#include <stdlib.h>
#include "ct_threadpool.h"
 

struct param{
    int id;
};
void *demo(void *arg){
     struct param *p = (struct param*)arg;
     printf("thread[%lu] execute task %d\n",pthread_self(),p->id);
}
 
int main(int argc, char **argv){
    /*添加任务*/
    int i=0;
    struct param *p = NULL;
    CtThreadPool *tp = new CtThreadPool(100);
    for(i=0;i<100;i++){
        p = (struct param*) malloc(sizeof(struct param));
        p->id =i;
        tp->thread_pool_add_worker(demo,(void*)p);
        //free(p);
    }
    printf("test1 ...\n");  
    tp->thread_pool_wait();
    //delete tp;
    //thread_pool_destory(thread_pool);
    return 0;
}
