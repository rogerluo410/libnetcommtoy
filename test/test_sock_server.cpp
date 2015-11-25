#include <iostream>
#include "sock.h"
#include "assert.h"
int main()
{
  int fd = Sock::Socket();
 
  //bool bBind = Sock::Bind(fd,"192.168.1.100",50000);
  //assert(bBind !=false);

  bool bSetBuf = Sock::SetSndBuf(fd,40960);
  assert(bSetBuf != false);

  bSetBuf = Sock::SetRcvBuf(fd,40960);
  assert(bSetBuf !=false);

  bool bSetTimeOut = Sock::SetSndTimeOut(fd,10);
  assert(bSetTimeOut !=false);

  bSetTimeOut = Sock::SetRcvTimeOut(fd,10);
  assert(bSetTimeOut !=false);

  int old_opt = Sock::SetNonBlock(fd);
  
  bool bSetNoDelay = Sock::SetNoDelay(fd);
  assert(bSetNoDelay != false);

  Sock::SetKeepAlive(fd);

  bool bBind = Sock::Bind(fd,"192.168.1.100",50000);
  assert(bBind !=false);

  int listen = Sock::Listen(fd,12);
  assert(listen > 0);

  char buffer[1024];
  while(1)
  {
    SOCKADDR_IN client_address;
    int fd_conn = Sock::Accept(fd,client_address);
    //assert(fd_conn > 0);
	if(fd_conn > 0)
	{
		std::cout<<"Accept successfully"<<std::endl;
		int recv = read(fd_conn,buffer,sizeof(buffer));
		std::cout<<"Read from sock["<<fd_conn<<"] with content {"<<buffer<<"}"<<std::endl;
        int sends = send(fd_conn,"ack",4,0);
		if(sends <= 0)
		{
			perror("send err");
			std::cout<<"Peer's writer is closed..."<<std::endl;
		}
		else
		{
			std::cout<<"send to peer successfully with ["<<sends<<"]"<<std::endl;
		}
		Sock::Shutdown_wr(fd_conn);
	}
	else
	{
		std::cout<<"Waitting for a connection"<<std::endl;
	}

	sleep(10);
  }

  return 0;
 

}
