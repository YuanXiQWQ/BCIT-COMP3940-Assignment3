#include "Socket.h"

#include <iostream>
// #include <sys/socket.h>
#include <sys/types.h>
// #include <resolv.h>
#include <sstream>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

using namespace std;

Socket::Socket(int sock)
{
	this->sock = sock;
  cout<<"created socket "<<sock<<endl;
}

char* Socket::getRequest() {
   int rval;
   char *buf = new char[2048];

   if ((rval = read(sock, buf, 2048)) < 0){
     perror("reading socket");
   }else  {
     printf("%s\n",buf);
   }

  return buf;
}

char* Socket::getRequestLine() {
  int rval;
  char *buf = new char[1024];
  char *pos = buf;
  int count = 0;

  char currChar;
  char lastChar;
  while ((rval = read(sock, pos, 1))) {
    if (rval < 1) {
      break;
    }
    currChar = *pos;
    if (lastChar == '\r' && currChar == '\n') {
      count++;
      break;
    }

    lastChar = currChar;
    count++;
    pos++;
  }

  char *outBuf = new char[count + 1];
  for (int i = 0; i < count; i++) {
    outBuf[i] = buf[i];
  }
  outBuf[count] = '\0';

  return outBuf;
}

int Socket::getReqFile(char *buf, int n) {
  cout << "in getReqFile" << endl;
  return readn(sock, buf, n);
}

int Socket::readn(int fd, void* buf, int n) {
  int nread;
  char *p = (char *)buf;
  char *q = (char *)buf + n;

  while (p < q) {
    if ((nread = read(fd, p, q - p)) < 0) {
      if (errno == EINTR)
        continue;
      else
        return -1;
    } else if (nread == 0)
      break;

    p += nread;
  }
  return p - (char *) buf;
}

void Socket::sendResponse(char *res){
  cout << "Sending response..." << endl;
  int rval;
  if ((rval = write(sock, res, strlen(res))) < 0){
    perror("writing socket");
  }else  {
    printf("%s\n",res);
  }
}

int Socket::getSock() {
  return sock;
}

void Socket::closeSocket() {
  cout<<"closing socket "<<sock<<endl;
  close(sock);
}

Socket::~Socket()
{
}
