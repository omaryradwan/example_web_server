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

#define GET 0


struct http_info {
  char* host;
  char* action;
  char* directory;
  char* version;
};
//////////Borrowed function from https://www.strudel.org.uk/itoa/
char* itoa(int val, int base){
  static char buf[32] = {0};
  int i = 30;
  for(; val && i ; --i, val /= base)
    buf[i] = "0123456789abcdef"[val % base];
  return &buf[i+1];
}

void fatal_err(char* err_disc, int type){
  perror(err_disc);
  close(connfd);
  close(listenfd);
  exit(type);
}


void contrlCHandle(int sig_num)
{
  signal(SIGINT, contrlCHandle);
  printf("terminating, while closing sockets\n");
  fflush(stdout);
  close(connfd);
  close(listenfd);
  exit(1);
}
void marker(int line){
  fprintf(stdout, "Marker: %i\n", line);
}

void val_report(char *disc, int val){
  fprintf(stdout, "%s: %i\n", disc, val);
}

char ** word_parse (char* buf){
  char **word_list = malloc (sizeof (char*));
  if (word_list == NULL) fatal_err ("BAD MALLOC", 1);
  int word_list_size = 0;
  int char_len = strlen (buf);
  val_report("char_len", char_len);
  marker (1);
  char* temp_word = malloc (sizeof (char*) * MAXLINE);
  marker (2);
  for (int i = 0; i < char_len; i++){
    if (buf [i] != ' ' && buf [i] != '\n' ){
      char chr = buf [i];
      strncat (temp_word, &chr, 1);
    }
    else {
      word_list_size++;
      word_list = realloc (word_list, word_list_size * sizeof (char *));
      marker (3);
      val_report ("word_list_size", word_list_size);
      val_report ("strlen(temp_word)", strlen (temp_word));
      word_list [word_list_size - 1] = realloc (word_list [word_list_size - 1], (strlen (temp_word + 1)) * sizeof (char));
      marker (4);
      strcpy (word_list [word_list_size - 1], temp_word);

      //printf ("Wordlist[%i]: %s\n", word_list_size - 1, word_list [word_list_size - 1]);
      memset (temp_word, '\0', strlen (temp_word));
    }
  }

  return word_list;
}

char* check_sanity (char* buf){
  char** word_list = word_parse (buf );
  if (strncmp (word_list [0], "GET", 3) != 0) fatal_err ("NO GET REQUEST, NO SUPPORT FOR OTHER FUNCTION", 1);
  else val_report ("Success, found GET request", 0);

  char* requested_directory = word_list [1];
  char* version = word_list [2];
  fprintf (stdout, "VERSION: %s\n", version);
  requested_directory = requested_directory + sizeof (char);
  fprintf (stdout, "%s is the requested directory\n", requested_directory);
  char* http_header = "HTTP/1.1 200 OK\nContent-Type: html\nContent Length: ";
  char* response = malloc (strlen (http_header) * sizeof (char));
  strcat ( response ,http_header);
  int file_fd = open (requested_directory, O_RDONLY);
  printf ("Fd of requested file: %i\n", file_fd);
  if (file_fd > 0){
    char tmp;
    char* file = malloc (sizeof (char));
    int file_size = 0;
    *file = '\0';

    while (read (file_fd, &tmp, 1) > 0){
      file_size++;
      file = realloc (file, (file_size + 1) * sizeof (char));
      strncat (file, &tmp, 1);

    }
    //fprintf (stdout, "%s\n", file);
    char* string_size = itoa (file_size, 10);
    char* double_endline = "\n\n";
    fprintf (stdout, "************MSG*****************\n%s%s%s%s**************ENDMSG*****************\n", response, string_size, double_endline, file);

    //   response = realloc (response, (strlen (response) + strlen (file) + strlen (string_size) +  2) * sizeof (char));
    response = realloc (response, (strlen (response) + strlen (string_size)) * sizeof (char));
    strcat (response, string_size);
    response = realloc (response, (strlen (response) + strlen (double_endline)) * sizeof (char));
    strncat (response, double_endline,2);
    printf ("strlen of file: %li compared to %s\n", strlen (file), string_size);
    /* response = realloc (response, (strlen (response) + strlen (file)) * sizeof (char)); */
    /* strcat (response, file); */

    char* http_file = malloc (sizeof (char) * (strlen (response) + strlen (file)));
    strcat (http_file, response);
    strcat (http_file, file);

    printf ("*************************************HTML RESPONSE\n");
    fprintf (stdout, "%s", http_file);
    printf ("*************************************HTML RESPONSE\n");
    free (response);
    free (file);
    free (word_list);
    return http_file;

  }
  else return NULL;
}

int main(int argc, char **argv){
  printf("Hello World\n");
  if(argc != 2) fatal_err("USAGE: ./webserver <desired port #>", -1);
  signal(SIGINT,contrlCHandle);
  val_report("Argv[1]", atoi(argv[1]));
  int port = atoi(argv[1]);
  if(port <= 1024) fatal_err("Bad port number, please choose port higher than 1024", 1);

  int listenfd, connfd, n;
  socklen_t clilen;
  char buffer[MAXLINE];
  struct sockaddr_in cliaddr, server;
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(port);
  memset(server.sin_zero, '\0', sizeof(server.sin_zero));

  if(bind(listenfd, (struct sockaddr *) &server, sizeof(server)) == -1)    fatal_err("Bind failed",1);
  if(listen(listenfd, LISTENQ) == -1) fatal_err("listen failed", 1);

  val_report("running server now", 1);

  while(1) {
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
    val_report("######################################Received Request##############################", 1);
    while((n = read(connfd, buffer, MAXLINE)) > 0) {
      puts(buffer);
      char* response = check_sanity (buffer);
      if (response != NULL){
        write (connfd, response, strlen (response));
        close (connfd);
        memset(buffer, '\0', MAXLINE);
        free (response);
      }
      else{
        close (connfd);
        memset(buffer, '\0', MAXLINE);
      }
    }
  }
}
