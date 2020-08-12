/* Name: Omar Radwan */
/* UID: 205105562 */
/* Class: CS118 Spring 2020 */
/* Project: Project 1 */
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
void val_report(char *disc, int val){
  fprintf(stdout, "%s: %i\n", disc, val);
}

int file_type(char* filename){
  char* type;
  int ext = -134;
  type = strrchr(filename, '.');
  if(type == NULL) return 4;
  type = type + 1;
  if (!type) {
    ext = 4;
  } else {
    //printf("type extension is %s\n", type);
    if(strncmp(type, "html", strlen("html")) == 0) ext = 0;
    else if(strncmp(type, "txt", strlen("txt")) == 0) ext = 1;
    else if(strncmp(type, "png", strlen("png")) == 0) ext = 2;
    else if(strncmp(type, "jpg", strlen("jpg")) == 0) ext = 3;
    else if(strncmp(type, "jpeg", strlen("jpeg")) == 0) ext = 3;
    else if(strncmp(type, "", strlen("")) == 0) ext = 4;
    else ext = 4;
  }
  val_report("extention type for file", ext);
  return ext;
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



//takes buffer of words, and makes it into a array of strings, returns char**
char** request_parse (char* buf, int* word_list_size){
  *word_list_size = 0;
  /* char **word_list = malloc (sizeof (char*)); */
  /* //char *word_list[MAXLINE]; */
  /* printf("****************MSG************************\n%s*******************************\n", buf); */
  /* if (word_list == NULL) fatal_err ("BAD MALLOC", 1); */
  /* //int word_list_size = 0; */
  /* int char_len = strlen (buf); */
  /* //xo  val_report("char_len", char_len); */
  /* char* temp_word = malloc (sizeof (char*) * MAXLINE); */


  /* memset(temp_word, '\0', MAXLINE); */

  /* for (int i = 0; i < char_len ; i++){ */
  /*   if(*word_list_size > 10) break;//we don't need to go any further tbh */
  /*   if (buf [i] != ' ' && buf [i] != '\n' ){ */
  /*     char chr = buf [i]; */
  /*     strncat (temp_word, &chr, 1); */
  /*   } */
  /*   else { */

  /*     *word_list_size = *word_list_size + 1; */
  /*     //      marker(11); */
  /*     //word_list = realloc (word_list, sizeof(char *)* *word_list_size); */
  /*     if(word_list != NULL )word_list = realloc (word_list, *word_list_size * sizeof (char *)); */
  /*     else word_list = malloc(*word_list_size * sizeof(char*)); */
  /*     //      marker(12); */
  /*     //val_report("Wordlist_size", *word_list_size); */
  /*     word_list [*word_list_size - 1] = malloc((strlen (temp_word)) * sizeof (char)); */
  /*     strcpy (word_list [*word_list_size - 1], temp_word); */

  /*     //printf ("Wordlist[%i]: %s\n", *word_list_size - 1, word_list [*word_list_size - 1]); */
  /*     memset (temp_word, '\0', strlen (temp_word)); */

  /*   } */
  /* } */

  /* free(temp_word); */
  char** word_list = malloc(sizeof(char*));
  char* delimiters = " \n\r";
  char* token = strtok(buf, delimiters);
  //  val_report(buf, 99);

  while(token != NULL){
    (*word_list_size)++;
    word_list = realloc(word_list, sizeof(char*) * *word_list_size);
    word_list[*word_list_size - 1] = malloc(strlen(token) * sizeof(char));
    memset(word_list[*word_list_size - 1], '\0', strlen(token) + 1);
    strncpy(word_list[*word_list_size - 1], token, strlen(token));
    ///val_report(token, 1);
    token = strtok(NULL, delimiters);
  }

  /* val_report("*@#@($(&@(#*&#(*&@(*&@# TOKEN STUFF @#(@#&(*@$@(&#($^(*@#&)))", 1); */
  /* for(int i  = 0; i < *word_list_size; i++) */
  /*   val_report(word_list[i], i); */
  /* val_report("*@#@($(&@(#*&#(*&@(*&@# TOKEN STUFF @#(@#&(*@$@(&#($^(*@#&)))", 1); */

  return word_list;
}



char* return_filename(char* buf){
  //  char** word_list = word_parse (buf );
  char **word_list;
  int* word_list_size = malloc(sizeof(int));
  *word_list_size = 0;
  word_list = request_parse(buf, word_list_size);
  //  for(int i = 0; i < *word_list_size; i++)
  //    printf("%s\n", word_list[i]);

  if (strncmp (word_list [0], "GET", 3) != 0) fatal_err ("NO GET REQUEST, NO SUPPORT FOR OTHER FUNCTION", 1);
  else val_report ("Success, found GET request", 0);
  word_list[1] = word_list[1] + sizeof(char);
  //  printf("word: %s\tsize: %li\n", word_list[1], strlen(word_list[1]));
  char* requested_directory = malloc(strlen(word_list[1]) * sizeof(char) + sizeof(char));


  strncpy(requested_directory, word_list[1], strlen(word_list[1]));
  requested_directory[strlen(word_list[1])] = '\0';
  char* version = word_list [2];
  fprintf(stdout, "&&&&&&&&HEADER INFO &&&&&&&&\n");
  fprintf (stdout, "VERSION: %s\n", version);
  fprintf (stdout, "%s is the requested file\tsize: %li\n", requested_directory, strlen(requested_directory));
  fprintf(stdout, "&&&&&&&&HEADER INFO &&&&&&&&\n");
  /* for(int i = 0; i < *word_list_size; i++) */
  /*   free(word_list[i]); //we'll free requestion directory ltr */
  free(word_list);
  free(word_list_size);
  if(access( requested_directory, F_OK ) != -1) return requested_directory;
  else {free(requested_directory); return NULL;}
}




char* generate_html_response(char* filename, int ext){
  char* http_header = NULL;
  if(ext == 0 )http_header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
  else if(ext == 1) http_header = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ";

  int file_fd = open (filename, O_RDONLY);
  //  printf ("Fd of requested file: %i\n", file_fd);
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
  //fprintf (stdout, "************MSG*****************\n%s%s%s%s**************ENDMSG*****************\n", http_header, string_size, double_endline, file);
  int message_size = strlen(http_header) + strlen(file) + strlen(double_endline) + strlen(string_size)  + strlen(file);
  val_report("strlen of orig", message_size);
  char* message = malloc(sizeof(char) * message_size);

  memset(message, 0, message_size);
  strcat(message, http_header);
  strcat(message, string_size);
  strcat(message, double_endline);
  strcat(message, file);

  printf ("*************************************HTML RESPONSE\n");
  fprintf (stdout, "%s", message);
  printf ("*************************************HTML RESPONSE\n");


  val_report("strlen of message", strlen(message));
  write(connfd, message, strlen(message));
  return message;
}

int generate_pic_response(char* filename, int ext, int connfd){
  char* http_header = NULL;

  if(ext == PNG )http_header = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: ";
  if(ext == JPG) http_header = "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nContent-Length: ";
  /* if(ext == PNG )http_header = "HTTP/1.1 200 OK\nContent-Type: application/octet-stream\nContent-Length: "; */
  /* if(ext == JPG) http_header = "HTTP/1.1 200 OK\nContent-Type: application/octet-stream\nContent-Length: "; */
  printf("PICTURE MODE\n");
  FILE* picture = fopen(filename, "r");
  if(picture == NULL) fatal_err("Cannot open", 1);
  fseek(picture, 0, SEEK_END);
  int picture_size = ftell(picture);
  char* file = malloc (sizeof (char));
  *file = '\0';
  char* string_size = itoa (picture_size, 10);
  fprintf(stdout, "size in string version: %s\n", string_size);
  char* double_endline = "\r\n\r\n";
  int message_size=  strlen(http_header) + strlen(file) + strlen(double_endline) + strlen(string_size);
  char* message = malloc(sizeof(char) * message_size);
  memset(message, 0, message_size);
  strcat(message, http_header);
  strcat(message, string_size);
  strcat(message, double_endline);
  printf("%s", message);
  val_report("Picture Size", picture_size);
  rewind(picture);
  char* buf = NULL;
  buf = malloc(sizeof(char) * picture_size);
  int remaining = fread(buf, sizeof(char), picture_size, picture);
  val_report("Bytes Sent", remaining);
  val_report("Sizeo buf", sizeof(buf));
  write(connfd, message, message_size);
  write(connfd, buf, picture_size);
  free(buf);
  //write(connfd,"\r\n\0", strlen("\r\n\0"));
  return 0;
}
int generate_bin_response(char* filename, int ext, int connfd){
  char* http_header = NULL;

  http_header = "HTTP/1.1 200 OK\nContent-Type: application/octet-stream\nContent-Length: ";
  printf("PICTURE MODE\n");
  FILE* picture = fopen(filename, "r");
  if(picture == NULL) fatal_err("Cannot open", 1);
  fseek(picture, 0, SEEK_END);
  int picture_size = ftell(picture);
  char* file = malloc (sizeof (char));
  *file = '\0';
  char* string_size = itoa (picture_size, 10);
  fprintf(stdout, "size in string version: %s\n", string_size);
  char* double_endline = "\r\n\r\n";
  int message_size=  strlen(http_header) + strlen(file) + strlen(double_endline) + strlen(string_size);
  char* message = malloc(sizeof(char) * message_size);
  memset(message, 0, message_size);
  strcat(message, http_header);
  strcat(message, string_size);
  strcat(message, double_endline);
  printf("%s", message);
  val_report("Binary File Size", picture_size);
  rewind(picture);
  char* buf = NULL;
  buf = malloc(sizeof(char) * picture_size);
  int remaining = fread(buf, sizeof(char), picture_size, picture);
  val_report("Bytes Sent", remaining);
  write(connfd, message, message_size);
  write(connfd, buf, picture_size);
  //write(connfd,"\r\n\0", strlen("\r\n\0"));
  free(buf);
  return 0;
}

int main(int argc, char **argv){
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
  int successful_responses = 0;
  while(1) {
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
    val_report("######################################Received Request##############################", 1);

    while((n = read(connfd, buffer, MAXLINE)) > 0) {

      char* file_requested = return_filename(buffer);
     //      val_report(file_requested, 3141512);

      if(file_requested == NULL) {
        //so we just ignore??
        printf("NOT FOUND, 404 Page\n");
        printf("404 NOT ENABLED, GO TO LINE 329 to enable\n");
        //TO INCLUDE 404 PAGE, UNCOMMENT THESE TWO LINES
        //char* message = generate_html_response("404.html", HTML);
        //write(connfd, message, strlen(message));
        close (connfd);
        memset(buffer, '\0', MAXLINE);
      }
      else { //if success
        int ext = file_type(file_requested);
        char* message = NULL;
        if(ext == HTML || ext == TXT) {
          memset(buffer, '\0', MAXLINE);
          successful_responses++;
          val_report("Successful responses", successful_responses);
          message = generate_html_response(file_requested, ext);
          write(connfd, message, strlen(message));
          close (connfd);

        }
        else if(ext == PNG || ext == JPG) {
          generate_pic_response(file_requested, ext, connfd);
          close(connfd);
        }
        else if(ext == ERR) val_report("BAD File extnsion, unsupporter", ext);
        else if(ext == BIN){
          generate_bin_response(file_requested, ext, connfd);
          close(connfd);
          //binary file shit up in this bitch
        }

      }
    }
  }
}
