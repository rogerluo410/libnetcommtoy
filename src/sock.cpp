#include "sock.h"

bool Sock::Name2IP(char *szName, char *szIP)
{
  hostent *host = ::gethostbyname(szName);
  if (!host) return false;

  char *szAddr = ::inet_ntoa(*(in_addr*)(host->h_addr));

  ::strcpy(szIP, szAddr);

  return true;
}

SOCKET Sock::Socket()
{
  SOCKET sockConnect = ::socket(AF_INET, SOCK_STREAM, 0);

  if (INVALID_SOCKET == sockConnect) return INVALID_SOCKET;

  int reuse = 1;

  ::setsockopt(sockConnect, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)); 

  return sockConnect;
}

bool Sock::Bind(SOCKET sock, const char *ip, unsigned short port)
{
  SOCKADDR_IN addr_in;
  bzero(&addr_in, sizeof(addr_in));
  addr_in.sin_family = AF_INET;
  addr_in.sin_port   = htons(port);
  in_addr_t addr     = inet_addr(ip);
  addr_in.sin_addr   = *((struct in_addr*)&addr);

  int nRet = ::bind(sock, (sockaddr*)&addr_in, sizeof(addr_in));
  if(SOCKET_ERROR == nRet) return false;

  return true;
}

void Sock::CloseSocket(SOCKET &sock)
{
  if(sock != INVALID_SOCKET)
  {
       close(sock);
       sock = INVALID_SOCKET;
  }
}

bool Sock::SetSndBuf(SOCKET sock, UINT nSize)
{
  int nBuf  = nSize;
  
  socklen_t nBufLen = sizeof(nBuf);

  int nRet = ::setsockopt(sock,SOL_SOCKET, SO_SNDBUF, &nBuf,nBufLen );

  if(SOCKET_ERROR == nRet) return false;

  nRet = ::getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &nBuf,&nBufLen);

  //std::cout << "nSize:"<<nSize <<",nBuf:"<<nBuf<<",nRet:"<<nRet<<std::endl;
  if(nSize * 2 != nBuf) return false;

  return true;  
}

bool Sock::SetRcvBuf(SOCKET sock, UINT nSize)
{
  int nBuf = nSize;
  socklen_t nBufLen = sizeof(nBuf);

  int nRet = ::setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &nBuf, nBufLen);
  if(SOCKET_ERROR == nRet) return false;

  nRet = ::getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &nBuf, &nBufLen );

  if(nSize * 2 != nBuf) return false;

  return true;
}

bool Sock::SetSndTimeOut(SOCKET sock, int nTimeOut)
{
   if (nTimeOut <= 0) nTimeOut = 1;
   struct timeval time;
   time.tv_sec = nTimeOut;

   int err = ::setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &time, (socklen_t)sizeof(time));

   if (0 != err)
   {
	   perror("Set send timeout");
	   return false;
   }
   return true;
}

bool Sock::SetRcvTimeOut(SOCKET sock, int nTimeOut)
{
  if (nTimeOut <= 0) nTimeOut = 1;

  struct timeval time;
  time.tv_sec = nTimeOut;

  int err = ::setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &time, (socklen_t)sizeof(time));
  if(0 != err)
  {
	  perror("Set receive timeout");
	  return false;
  }
  return true;
}

int Sock::SetNonBlock(SOCKET sock)
{
 int old_opt = fcntl(sock, F_GETFL);
 int new_opt = old_opt | O_NONBLOCK;
 fcntl(sock, F_SETFL, new_opt);
 return old_opt;
}

bool Sock::SetNoDelay(SOCKET sock)
{
  int nodelay = 1;	
  int err = ::setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,&nodelay,(socklen_t)sizeof(nodelay));
  if(0!=err) 
  {
   perror("Set no nagle");		
   return false;
  }

  return true;
}

SOCKET Sock::Listen(SOCKET sock, int maxConn)
{
 int nRet = ::listen(sock, maxConn); 
 if(SOCKET_ERROR == nRet)
 {
   CloseSocket(sock);
   return INVALID_SOCKET;
 } 
 return sock;
}

SOCKET Sock::Accept(SOCKET sock, SOCKADDR_IN &addr_from)
{
  socklen_t nLen= sizeof(SOCKADDR_IN);

  SOCKET sockAccept = INVALID_SOCKET;

  if(INVALID_SOCKET == sock) return INVALID_SOCKET;

  sockAccept = ::accept(sock, (sockaddr*)&addr_from, &nLen);

  if(INVALID_SOCKET == sockAccept) return INVALID_SOCKET;

  return sockAccept;
}

SOCKET Sock::Connect(const char *szServerIP, USHORT nServerPort, unsigned int nTimeOut, unsigned int nReconnTimes)
{
  SOCKADDR_IN address;
  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  inet_pton(AF_INET, szServerIP, &address.sin_addr);
  address.sin_port = htons(nServerPort);

  SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (INVALID_SOCKET == sockfd) return INVALID_SOCKET;

  struct timeval timeout;
  timeout.tv_sec  = nTimeOut;
  timeout.tv_usec = 0;
  socklen_t len   = sizeof(timeout);
  int nRet = setsockopt( sockfd , SOL_SOCKET, SO_SNDTIMEO, &timeout, len);
  if (nRet == SOCKET_ERROR) return SOCKET_ERROR;

  while ( nReconnTimes > 0 )
  {
    nRet = connect(sockfd, (sockaddr*)&address, sizeof(address) );
    if (nRet == SOCKET_ERROR)
    {
      if (errno == EINPROGRESS)
      {
        nReconnTimes --;
      }
      else
      {
		perror("Connection err");  
        return INVALID_SOCKET;
      }
    }
    else
    {
      break;
    }
  }

  return sockfd;    
}

SOCKET Sock::ConnectEx(const char *szServerIP, USHORT nServerPort, unsigned int nTimeOut)
{
  SOCKADDR_IN address;
  bzero(&address, sizeof(address));
  address.sin_family = AF_INET;
  inet_pton(AF_INET, szServerIP, &address.sin_addr);
  address.sin_port = htons(nServerPort);

  SOCKET sockfd = socket(AF_INET,SOCK_STREAM,0);
  if (INVALID_SOCKET == sockfd) return INVALID_SOCKET;  

  int old_option = SetNonBlock(sockfd);
   
  int nRet = ::connect(sockfd, (sockaddr*)&address, sizeof(address));
  if(nRet != 0)
  {
    if(errno == EAGAIN) return INVALID_SOCKET;
    if(errno == ETIMEDOUT) 
    {
      struct timeval timeout;
      timeout.tv_sec  = nTimeOut;
      timeout.tv_usec = 0;
 
      fd_set write;
      FD_ZERO(&write);
      FD_SET(sockfd, &write);
      nRet = select(0,0,&write,0, &timeout);
      if(nRet <= 0) return INVALID_SOCKET;
      if(FD_ISSET(sockfd,&write)) return sockfd; 
    }
  }
  
  return sockfd;
}

/*
bool Sock::SendData(SOCKET sock, void *pData, UINT nLen)
{
  bool bRet = true;
  if (!pData) return false;
  if (INVALID_SOCKET == sock) return false;  

  UINT nByteSent = 0;
  while (nLen > nByteSent)
  {
     int nSend = ::send(sock, pData+nByteSent, nLen - nByteSent, 0);
     if(nSend > 0) nByteSent += nSend;
  }
  return true; 
}

bool Sock::RecvData(SOCKET sock, void *pData, UINT nLen)
{
  bool bRet = true;
  if (!pData) return false;
  if (INVALID_SOCKET == sock) return false;

  UINT nByteSent = 0;
  while (nLen > nByteSent)
  {
    int nSend = recv(sock, pData+nByteSent, nLen - nByteSent, 0);
    if(nSend > 0) nByteSent += nSend;
  }
      return true;
}*/

void Sock::Shutdown_wr(SOCKET sock)
{
	::shutdown(sock,SHUT_WR);
}

void Sock::Shutdown_rd(SOCKET sock)
{
	::shutdown(sock, SHUT_RD);
}

void Sock::Shutdown_rdwr(SOCKET sock)
{
	::shutdown(sock,SHUT_RDWR);
}

void Sock::SetKeepAlive(SOCKET sock,int keepalive,int keepidle,int keepinterval,int keepcount)
{
  //int keepalive    = 1;  //Open keepalive property
  //int keepidle     = 6;  //Detected if no data communication in 60 secs  
  //int keepinterval = 5;  //Send msg timeval by 5 secs
  //int keepcount    = 1;  
  
  setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepalive, sizeof(keepalive) );
  setsockopt(sock, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle, sizeof(keepidle));
  setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, (void*)&keepinterval, sizeof(keepinterval));
  setsockopt(sock, SOL_TCP, TCP_KEEPCNT, (void*)&keepcount, sizeof(keepcount));
}
