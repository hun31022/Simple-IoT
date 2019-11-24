
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include "stems.h"
#include "/usr/include/mysql/mysql.h"

//
// This program is intended to help you test your web server.
// 

int main(int argc, char *argv[])
{
  //char *astr = "Currently, CGI program is running, but argument passing is not implemented.";  
  char *method, *bodyLen, *takeBody,  *entireBody, *restBody;
  char requestbody[MAXLINE], bodyForTok[MAXLINE], content[MAXLINE];
  int bodyLength, takeBodyLen, readB = 0;
  //rio_t rio;
  //Rio_readinitb(&rio, STDIN_FILENO);

  method = getenv("REQUEST_METHOD");
  bodyLen = getenv("CONTENT_LENGTH");
  bodyLength = atoi(bodyLen);
  takeBody = getenv("REQUEST_BODY");

  strcpy(requestbody, takeBody);
  takeBodyLen = strlen(takeBody);

  if(bodyLength > takeBodyLen) {
    entireBody = (char *)malloc(sizeof(char) * (bodyLength + 1));
    restBody = (char *)malloc(sizeof(char) * (bodyLength - takeBodyLen + 1));
    readB = read(STDIN_FILENO, restBody, bodyLength - takeBodyLen + 1);
    strcpy(entireBody, requestbody);
    strcat(entireBody, restBody);
    entireBody[strlen(entireBody)-1] = '\0';
    sprintf(content, "To confirm sending...%s\n", entireBody);
    free(entireBody);
    free(restBody);
  }
  else {
    requestbody[bodyLength - 1] = '\0';
    sprintf(content, "To confirm sending...%s\n", requestbody);
  }

  //Body Seperate
  char *token; 
  char name[MAXLINE], time[MAXLINE], valueC[MAXLINE], tempName[MAXLINE], tempTime[MAXLINE], tempValue[MAXLINE];
  float value;
  int count = 0;
  strcpy(bodyForTok, takeBody);
  token = strtok(bodyForTok, "&");
  while (token != NULL)
  {
    if (count == 0)
      strcpy(tempName, token);
    if (count == 1)
      strcpy(tempTime, token);
    if (count == 2)
      strcpy(tempValue, token);
    token = strtok(NULL, "&");
    count++;
  }

  //Part name
  token = strtok(tempName, "=");
  count = 0;
  while (token != NULL)
  {
    if (count == 1)
      strcpy(name, token);
    token = strtok(NULL, "=");
    count++;
  }
  // Part time
  token = strtok(tempTime, "=");
  count = 0;
  while (token != NULL)
  {
    if (count == 1)
      strcpy(time, token);
    token = strtok(NULL, "=");
    count++;
  }
  // Parkt value
  token = strtok(tempValue, "=");
  count = 0;
  while (token != NULL)
  {
    if (count == 1)
      strcpy(valueC, token);
    token = strtok(NULL, "=");
    count++;
  }
  valueC[strlen(valueC)-1] = '\0';
  value = atof(valueC);

  //db
  MYSQL conn;
  MYSQL_RES *res;
  MYSQL_ROW row;
  int id, dbId[100], dbCount[100], rowCount = -1, new = 0;
  char query[255], dbName[100][100];

  mysql_init(&conn); 

  if(!mysql_real_connect(&conn, NULL, "root", "0803", NULL, 3306, (char *)NULL, 0)) {
    sprintf(content, "%s%s\n", content, mysql_error(&conn));
    exit(1);
  }
  if(mysql_query(&conn, "USE sensor")) {
    sprintf(content, "%s%s\n", content, mysql_error(&conn));
    exit(1); 
  }

  if(mysql_query(&conn, "select * from sensorList")) {
    sprintf(content, "%s%s\n", content, mysql_error(&conn));
    exit(1); 
  }
  res = mysql_store_result(&conn);
  while( (row = mysql_fetch_row(res)) != NULL) {
    ++rowCount;
    strcpy(dbName[rowCount], row[0]);
    dbId[rowCount] = atoi(row[1]);
    dbCount[rowCount] = atoi(row[2]);
  }
  id = rowCount+2;
  if(rowCount < 0) { //first
    sprintf(query, "insert into sensorList values ('%s', %d, %d, %f);", name, id, 1, value);
    if(mysql_query(&conn, query)) {
      sprintf(content, "%s%s\n", content, mysql_error(&conn));
      exit(1); 
    }
    memset(query, 0, sizeof(query));
    sprintf(query, "create table sensorData01 (time varchar(50) not null, data float not null, seqnum int primary key);");
    if(mysql_query(&conn, query)) {
      sprintf(content, "%s%s\n", content, mysql_error(&conn));
      exit(1); 
    }
    memset(query, 0, sizeof(query));
    sprintf(query, "insert into sensorData01 values ('%s', %f, %d);", time, value, 1);
    if(mysql_query(&conn, query)) {
      sprintf(content, "%s%s\n", content, mysql_error(&conn));
      exit(1); 
    }
    memset(query, 0, sizeof(query));
  }
  else if(rowCount >= 0) {
    for(int i=0; i<=rowCount; i++) {
      if((strcmp(dbName[i], name))==0) {
        ++new;
        dbCount[i] += 1;
        sprintf(query, "update sensorList set count=%d, last_value=%f where id=%d;", dbCount[i], value, dbId[i]);
        if(mysql_query(&conn, query)) {
          sprintf(content, "%s%s\n", content, mysql_error(&conn));
          exit(1); 
        }
        memset(query, 0, sizeof(query));
        sprintf(query, "insert into sensorData%0.2d values ('%s', %f, %d);", dbId[i], time, value, dbCount[i]);
        if(mysql_query(&conn, query)) {
          sprintf(content, "%s%s\n", content, mysql_error(&conn));
          exit(1); 
        }
      }
    }
    if(new == 0) {
      sprintf(query, "insert into sensorList values ('%s', %d, %d, %f);", name, id, 1, value);
      if(mysql_query(&conn, query)) {
        sprintf(content, "%s%s\n", content, mysql_error(&conn));
        exit(1); 
      }
      memset(query, 0, sizeof(query));
      sprintf(query, "create table sensorData%0.2d (time varchar(50) not null, data float not null, seqnum int primary key);", id);
      if(mysql_query(&conn, query)) {
        sprintf(content, "%s%s\n", content, mysql_error(&conn));
        exit(1); 
      }
      memset(query, 0, sizeof(query));
      sprintf(query, "insert into sensorData%0.2d values ('%s', %f, %d);", id, time, value, 1);
      if(mysql_query(&conn, query)) {
        sprintf(content, "%s%s\n", content, mysql_error(&conn));
        exit(1); 
      }
    }
  }
  memset(query, 0, sizeof(query));
  mysql_free_result(res);
  mysql_close(&conn);
  sprintf(content, "%ssuccess save in database!(200)\n", content);

  int fd;

  mkfifo( "MYFIFO", 0666);
  if((fd = open( "MYFIFO", O_RDWR)) == -1) {
    printf("fail to call open()\n");
  }
  if(write(fd, bodyLen, sizeof(bodyLen)) == -1) {
    printf("fail to call write()1\n");
  }
  if(write( fd, takeBody, strlen( takeBody)) == -1) {
    printf("fail to call write()2\n");
  }
  close(fd);

  printf("HTTP/1.0 200 OK\r\n");
  //printf("Server: My Web Server\r\n");
  printf("Content-Length: %d\r\n", (int)strlen(content));
  printf("Content-Type: text/plain\r\n\r\n");
  printf("%s", content);
  fflush(stdout);

  return(0);
}