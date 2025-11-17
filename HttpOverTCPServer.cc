//use localhost on windows to access localhost of ubunto
#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
main() {
  int sock, length;
  struct sockaddr_in server;
  int msgsock;
  char buf[1024];
  int rval;
  int i;
  
  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("opening stream socket");
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(8888);
  
  if (bind (sock, (struct sockaddr *)&server, sizeof server) < 0) {
    perror ("binding stream socket");
  }
  listen (sock, 5);
  msgsock = accept(sock, (struct sockaddr *)0, (socklen_t *)0);
  if (msgsock == -1) {
    perror("accept");
  }
  if ((rval = read(msgsock, buf, 1024)) < 0){
    perror("reading socket");
  }else  {
    printf("%s\n",buf);
  }
  strcpy(buf,"HTTP/1.1 200 OK\r\nContent-length: 20\r\nContent-type: text/plain\r\n\r\ntemp1.txt, temp2.txt");
  if ((rval = write(msgsock, buf, 1024)) < 0){
    perror("writing socket");
  }

  close (msgsock);
}
