#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#define LISTENQ 1024
#define MAXLINE 4096
int connfd, listenfd;
#define ERR -1
#define HTML 0
#define TXT 1
#define JPG 2
#define PNG 3
#define BIN 4


int main(){
  FILE* fd = fopen("paris.jpg", "r");
  fseek(fd, 0, SEEK_END);
  int pic_size = ftell(fd);
  printf("File size: %i\n", pic_size);
  rewind(fd);
  char* buf = malloc(sizeof(char) * pic_size);
  fread(buf, sizeof(char), pic_size, fd);
  for(int i = 0; i < pic_size; i++){
    printf("%c", buf[i]);
  }
  fclose(fd);
}
