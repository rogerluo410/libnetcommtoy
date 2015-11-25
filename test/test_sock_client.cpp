#include <iostream>
#include "sock.h"
#include "assert.h"


int main()
{
  // SOCKADDR_IN address;
  // bzero(&address,sizeof(address));
  // address.sin_family = AF_INET;
  // inet_pton(AF_INET,"192.168.1.100",&address.sin_addr);
  // address.sin_port = htons(50000);
 
  // int sockfd = Sock::Socket();
  // assert(sockfd > 0);
   int sockfd = Sock::Connect("192.168.1.100",50000,5,3);
   assert(sockfd > 0);
   Sock::SetNonBlock(sockfd);
   
   send(sockfd,"Connected",10,0);
   Sock::Shutdown_wr(sockfd);
   std::cout<<"Test would block by send,if fd isn't NON-BLOCK 1..."<<std::endl;
   
   char buffer[1024];
   int rc = recv(sockfd,buffer,sizeof(buffer),0);
   if(rc <= 0 )
   {
	 perror("recv err");
	 Sock::Shutdown_rd(sockfd);
   }
   else
	  std::cout<<"read {"<<buffer<<"}"<<std::endl;
   std::cout<<"Test would block by recv, if fd isn't NON-BLOCK  2..."<<std::endl;
   
   close(sockfd);

   sleep(10);

   int sockfd1 = Sock::ConnectEx("192.168.1.100",50000,5);
   assert(sockfd1 > 0);
   Sock::SetNonBlock(sockfd1);

   send(sockfd1,"Connected by V2",16,0);
   Sock::Shutdown_wr(sockfd);

   int rc1 = recv(sockfd1,buffer,sizeof(buffer),0);
   if(rc1 <= 0)
   {
	   perror("recv err");
	   Sock::Shutdown_rd(sockfd);
   }
   else
		std::cout<<"read {"<<buffer<<"}"<<std::endl;

    close(sockfd);

    return 0;
}
