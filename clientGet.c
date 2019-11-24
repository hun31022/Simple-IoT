/*
 * clientGet.c: A very, very primitive HTTP client for console.
 * 
 * To run, prepare config-cg.txt and try: 
 *      ./clientGet
 *
 * Sends one HTTP request to the specified HTTP server.
 * Prints out the HTTP response.
 *
 * For testing your server, you will want to modify this client.  
 *
 * When we test your server, we will be using modifications to this client.
 *
 */


#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include "stems.h"

/*
 * Send an HTTP request for the specified file 
 */
void printFirstMenu(void)
{
  printf("-------------------------------------------------\n");
  printf("If you want to see commands, type 'help' \n");
  printf("You must input commands correctly! \n");
  printf("-------------------------------------------------\n");
}

void printMenu(void)
{
  printf("=================================================\n");
}

void printHelp(void)
{
  printf(" help             : list available commands. \n");
  printf(" list             : print list of sensors. \n");
  printf(" info <sname>     : print data of sensor <sname>. \n");
  printf(" get <sname>      : recent(time,value) print of sensor <sname>. \n");
  printf(" get <sname ><n>  : recent(time,value) <n> print of sensor <sname>. \n");
  printf(" quit             : quit the program. \n");
}

void clientSend(int fd, char *filename)
{
  char buf[MAXLINE];
  char hostname[MAXLINE];

  Gethostname(hostname, MAXLINE);

  /* Form and send the HTTP request */
  sprintf(buf, "GET %s HTTP/1.1\n", filename);
  sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
  Rio_writen(fd, buf, strlen(buf));
}
  
/*
 * Read the HTTP response and print it out
 */
void clientPrint(int fd)
{
  rio_t rio;
  char buf[MAXBUF];  
  int length = 0;
  int n;
  
  Rio_readinitb(&rio, fd);

  /* Read and display the HTTP Header */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (strcmp(buf, "\r\n") && (n > 0)) {
    printf("Header: %s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);

    /* If you want to look for certain HTTP tags... */
    if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
      printf("Length = %d\n", length);
    }
  }

  /* Read and display the HTTP Body */
  n = Rio_readlineb(&rio, buf, MAXBUF);
  while (n > 0) {
    printf("%s", buf);
    n = Rio_readlineb(&rio, buf, MAXBUF);
  }
}

/* currently, there is no loop. I will add loop later */
void userTask(char hostname[], int port, char webaddr[])
{
  int clientfd;

  clientfd = Open_clientfd(hostname, port);
  clientSend(clientfd, webaddr);
  clientPrint(clientfd);
  Close(clientfd);
}

void getargs_cg(char hostname[], int *port, char webaddr[])
{
  FILE *fp;

  fp = fopen("config-cg.txt", "r");
  if (fp == NULL)
    unix_error("config-cg.txt file does not open.");

  fscanf(fp, "%s", hostname);
  fscanf(fp, "%d", port);
  fscanf(fp, "%s", webaddr);
  fclose(fp);
}

int main(void)
{
  // alarm server
  pid_t pid;
  pid = Fork();
  if(pid==0) { 	
    Execve("./pushServer", NULL, environ);
  }
  else if(pid>0) {
    char hostname[MAXLINE], webaddr[MAXLINE], addr[MAXLINE], cmd[MAXLINE], value[MAXLINE], count[MAXLINE], cmdTmp[MAXLINE];
    int port;
    char *command[5] = { "help", "list", "info", "get" , "quit"};
    char inputC[MAXLINE];
    char *strTmp;

    getargs_cg(hostname, &port, webaddr);
    //userTask(hostname, port, webaddr);
    
    strTmp = strtok(webaddr, "?");
    strcpy(addr, strTmp);
    printFirstMenu();
    do {
      do {
        printMenu();
        printf("# ");
        fgets(inputC, sizeof(inputC), stdin);
        if(inputC[strlen(inputC) - 1] == '\n')  //remove '\n'
          inputC[strlen(inputC) - 1] = '\0';        
      } while (!strstr(inputC, command[0]) &&
        !strstr(inputC, command[1]) &&
        !strstr(inputC, command[2]) &&
        !strstr(inputC, command[3]) && 
        !strstr(inputC, command[4]));
      if (strstr(inputC, command[0]))  //help
        printHelp();
      else if (strstr(inputC, command[1])) { //list
        sprintf(cmd, "command=%s", command[1]);
        sprintf(webaddr, "%s?%s", addr, cmd);
        userTask(hostname, port, webaddr);
        //printf("%s\n", webaddr);
      } 
      else if (strstr(inputC, command[2])) {  //info
        strTmp = strtok(inputC, " ");
        strTmp = strtok(NULL, "");
        strcpy(value, strTmp);
        sprintf(cmd, "command=%s&value=%s", command[2], value);
        sprintf(webaddr, "%s?%s", addr, cmd);
        userTask(hostname, port, webaddr);
      }
      else if (strstr(inputC, command[3])) {  //get
        strTmp = strtok(inputC, " ");
        strTmp = strtok(NULL, "");
        if (!strstr(strTmp, " ")) {
          strcpy(value, strTmp);          
          sprintf(cmd, "command=%s&value=%s", command[3], value);
          sprintf(webaddr, "%s?%s", addr, cmd);
          userTask(hostname, port, webaddr);
        }
        else {
          strcpy(cmdTmp, strTmp);
          strTmp = strtok(cmdTmp, " ");
          strcpy(value, strTmp);
          strTmp = strtok(NULL, "");
          strcpy(count, strTmp);
          sprintf(cmd, "command=%s&value=%s&count=%s", command[3], value, count);
          sprintf(webaddr, "%s?%s", addr, cmd);
          userTask(hostname, port, webaddr);  
          //printf("%s\n", webaddr);        
        }
      }
    } while (!(strstr(inputC, command[4])));
     wait(NULL);
  }

  return(0);
}


