//If using WSL2 then run "ip route" on ubunto window to get the IP address

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

main() {
  int sock;
  struct sockaddr_in server;
  int msgsock;
  char buf[1024];
  struct hostent *hp;
  const char *host = "172.29.144.1";
  int rval;

  sock = socket (AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("opening stream socket");
  }

  bzero(&server, sizeof(server));
  hp = gethostbyname("localhost");
  bcopy((char*)hp->h_addr, (char*)&server.sin_addr, hp->h_length);
  inet_pton(AF_INET, host, &server.sin_addr);

  server.sin_family = AF_INET;
  server.sin_port = htons(8081);
  
  if (connect(sock, (struct sockaddr*)&server, sizeof(server))<0){
    perror("connecting");
  }

  strcpy(buf,"GET /midp/hits HTTP/1.0\n\r\n\r");
  if ((rval = write(sock, buf, 1024)) < 0){
    perror("writing socket");
  }
if ((rval = read(sock, buf, 1024)) < 0){
    perror("reading socket");
  }
	printf("%s\n", buf);
  close (sock);
}
