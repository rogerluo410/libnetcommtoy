#pragma once
typedef int            SOCKET;
typedef unsigned int   UINT;
typedef unsigned short USHORT;
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#define SOCKADDR_IN struct sockaddr_in

#include <iostream>
#include <sys/types.h>    
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

class Sock
{
public:
  
     //Create socket
     static SOCKET Socket();

     //Bind a port
     static bool Bind(SOCKET sock, const char *ip, unsigned short port);

     //Convert domain to IP address
     static bool Name2IP(char * szName, char *szIP);

     //Set buffer size
     static bool SetSndBuf(SOCKET sock, UINT nSize);
     static bool SetRcvBuf(SOCKET sock, UINT nSize);

      //Set timeout 
      static bool SetSndTimeOut(SOCKET sock, int nTimeOut);
      static bool SetRcvTimeOut(SOCKET sock, int nTimeOut); 

      //Set non-block mode
      static int SetNonBlock(SOCKET sock);

	  //Set no delay
	  static bool SetNoDelay(SOCKET sock);

      //Listen
      static SOCKET Listen(SOCKET sock, int maxConn);

      //Accept connections
      static SOCKET Accept(SOCKET sockListen, SOCKADDR_IN &addr_from);

      //Positive connection
      static SOCKET Connect(const char *szServerIP, USHORT nServerPort, unsigned int nTimeOut,unsigned int nReconnTimes = 5);
      static SOCKET ConnectEx(const char *szServerIP,USHORT nServerPort,unsigned int nTimeOut);

      /*
      //Send data
      static bool SendData(SOCKET sock, void *pData, UINT nLen);

      //Receive data
      static bool RecvData(SOCKET sock, void *pData, UINT nLen);
	  */

	  //Shutdown writer
	  static void Shutdown_wr(SOCKET sock);

	  //Shutdown reader
	  static void Shutdown_rd(SOCKET sock);

	  //Shutdown all
	  static void Shutdown_rdwr(SOCKET sock);

      //Close socket
      static void CloseSocket(SOCKET &sock);
  
      //Set keep alive
      static void SetKeepAlive(SOCKET sock,int keepalive =1,int keepidle =6,
	  						 int keepinterval =5,int keepcount =1);   
};


