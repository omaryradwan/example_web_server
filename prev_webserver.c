#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#define LISTENQ         1024
#define MAXLINE 4096
#define SERV_PORT 9000

int main(int argc, char **argv){
  int listenfd, connfd, n;
  pid_t childpid;
  socklen_t clilen;
  char buf[MAXLINE];
  struct sockaddr_in cliaddr, servaddr;
  char user_command[MAXLINE];
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(SERV_PORT);
  memset(servaddr.sin_zero, '\0', sizeof(servaddr.sin_zero));

  if(bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1){
    perror("Bind");
    exit(1);
  }
  if(listen(listenfd, LISTENQ) == -1) {
    perror("listen");
    exit(1);
    
  }

  fprintf(stdout, "Server Running, waiitng for connection\n");
  for(;;){
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &clilen);
    fprintf(stdout, "Received reqest\n");
    while((n = recv(connfd, buf, MAXLINE, 0)) > 0) {
      printf("String received from and resent to the client:");
      puts(buf);
      send(connfd, buf, n, 0);
      if(strcmp(buf, "q\n") == 0){
        printf("quit\n");
        break;
        
      }
      memset(buf, '\0', MAXLINE);
      
    }
    if(n < 0){
      perror("READ ERROR");
      exit(1);
      
    }
    printf("close connection...fd\n");
    close(connfd);
    
  }
  printf("closing listening socket\n");
  close(listenfd);
}