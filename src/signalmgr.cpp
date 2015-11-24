#include "signalmgr.h"

CtSignalMgr::CtSignalMgr()
{
	sig_set = new int[SIG_SET_SIZE];
	index = 0;
	sigemptyset(&(this->mask_set));
}

CtSignalMgr::~CtSignalMgr()
{
	delete sig_set;
}

int CtSignalMgr::add_set(int *sig_set,int N,__sighandler_t default_handler)
{
  	for(int i=0;i<N; i++ )
	{
		this->sig_set[this->index++] = sig_set[i];
		this->register_signal(sig_set[i],default_handler);
	}
	return 0;
}

int CtSignalMgr::register_signal(int signo,__sighandler_t handler)
{
	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_flags |= SA_RESTART;
	sa.sa_handler = handler;
	sigaddset(&(this->mask_set),signo);
	return sigaction(signo,&sa,NULL);
}

int CtSignalMgr::suspend_signal(int signo)
{
	return	sigdelset(&(this->mask_set),signo);
}
