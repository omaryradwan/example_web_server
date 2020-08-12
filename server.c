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


//takes buffer of words, and makes it into a array of strings, returns char**
void  header_parse (char* buf, char** word_list, int* word_list_size){
  //char **word_list = malloc (sizeof (char*));
  if (word_list == NULL) fatal_err ("BAD MALLOC", 1);
  //int word_list_size = 0;
  int char_len = strlen (buf);
  val_report("char_len", char_len);
  char* temp_word = malloc (sizeof (char*) * MAXLINE);
  for (int i = 0; i < char_len; i++){
    if (buf [i] != ' ' && buf [i] != '\n' ){
      char chr = buf [i];
      strncat (temp_word, &chr, 1);
    }
    else {
      *word_list_size = *word_list_size + 1;
      word_list = realloc (word_list, *word_list_size * sizeof (char *));
      val_report ("word_list_size", *word_list_size);
      val_report ("strlen(temp_word)", strlen (temp_word));
      word_list [*word_list_size - 1] = realloc (word_list [*word_list_size - 1], (strlen (temp_word + 1)) * sizeof (char));
      strcpy (word_list [*word_list_size - 1], temp_word);

      //printf ("Wordlist[%i]: %s\n", word_list_size - 1, word_list [word_list_size - 1]);
      memset (temp_word, '\0', strlen (temp_word));
    }
  }
  free(temp_word);
}



char* return_filename(char* buf){
  //  char** word_list = word_parse (buf );
  char **word_list = malloc(sizeof(char*));
  int* word_list_size = malloc(sizeof(int));
  header_parse(buf, word_list, word_list_size);
  if (strncmp (word_list [0], "GET", 3) != 0) fatal_err ("NO GET REQUEST, NO SUPPORT FOR OTHER FUNCTION", 1);
  else val_report ("Success, found GET request", 0);
  char* requested_directory = word_list [1];
  char* version = word_list [2];
  fprintf (stdout, "VERSION: %s\n", version);
  requested_directory = requested_directory + sizeof (char);
  fprintf (stdout, "%s is the requested directory\n", requested_directory);
  for(int i = 0; i < *word_list_size; i++)
    if(i != 1)free(word_list[i]); //we'll free requestion directory ltr
  free(word_list);
  if(access( requested_directory, F_OK ) != -1) return requested_directory; //SO
  else return NULL;
}

char* generate_html_response(char* filename){
  char* http_header = "HTTP/1.1 200 OK\nContent-Type: html\nContent Length: ";
  char* response = malloc (strlen (http_header) * sizeof (char));
  strcat ( response ,http_header);
  int file_fd = open (filename, O_RDONLY);
  printf ("Fd of requested file: %i\n", file_fd);
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
  response = realloc (response, (strlen (response) + strlen (string_size)) * sizeof (char));
  strcat (response, string_size);
  response = realloc (response, (strlen (response) + strlen (double_endline)) * sizeof (char));
  strncat (response, double_endline,2);
  printf ("strlen of file: %li compared to %s\n", strlen (file), string_size);
  char* http_file = malloc (sizeof (char) * (strlen (response) + strlen (file)));
  strcat (http_file, response);
  strcat (http_file, file);
  printf ("*************************************HTML RESPONSE\n");
  fprintf (stdout, "%s", http_file);
  printf ("*************************************HTML RESPONSE\n");
  return http_file;
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
      char* file_requested = return_filename(buffer);
      if(file_requested == NULL) {
        close (connfd);
        memset(buffer, '\0', MAXLINE);
      }
      else {
        char* message = generate_html_response(file_requested);
        write(connfd, message, strlen(message));
      }
    }
  }
}
