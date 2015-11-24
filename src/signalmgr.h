#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define SIG_SET_SIZE 50

class CtSignalMgr
{
public:
	CtSignalMgr();
	~CtSignalMgr();
	int add_set(int *sig_set ,int N , __sighandler_t default_handler = SIG_IGN);
	int register_signal(int signo,__sighandler_t handler);
	int suspend_signal(int signo);

private:
	int *sig_set;
    sigset_t mask_set;
	int index;
};
