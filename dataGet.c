#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>
#include "/usr/include/mysql/mysql.h"

//
// This program is intended to help you test your web server.
// You can use it to test that you are correctly having multiple 
// threads handling http requests.
//
// htmlReturn() is used if client program is a general web client
// program like Google Chrome. textReturn() is used for a client
// program in a embedded system.
//
// Standalone test:
// # export QUERY_STRING="name=temperature&time=3003.2&value=33.0"
// # ./dataGet.cgi

void databaseReturn(void)
{
  char content[MAXLINE], cmd[MAXLINE], value[MAXLINE], count[MAXLINE];
  char *buf, *ptr, *strTmp;
  char *command[5] = { "list", "info", "get" };
  int realCount, cnt = 0;

  buf = getenv("QUERY_STRING");
  ptr = strsep(&buf, "&");
  while (ptr != NULL){
    if(cnt == 0) {
      strTmp = strtok(ptr, "=");
      strTmp = strtok(NULL, "");
      strcpy(cmd, strTmp);
    }
    else if(cnt == 1) {
      strTmp = strtok(ptr, "=");
      strTmp = strtok(NULL, "");
      strcpy(value, strTmp);      
    }
    else if(cnt == 2) {
      strTmp = strtok(ptr, "=");
      strTmp = strtok(NULL, "");
      strcpy(count, strTmp);      
    }    
    ++cnt;
    ptr = strsep(&buf, "&");
  }
  realCount = atoi(count);

  MYSQL conn;
  MYSQL_RES *res;
  MYSQL_ROW row;
  int id, idCount, rowCount = -1, i;
  char query[255], dbTime[100][100], pwd[100] = "0803", dbInfo[100];
  float dbVal[100];

  mysql_init(&conn);

  if(!mysql_real_connect(&conn, NULL, "root", pwd, NULL, 3306, (char *)NULL, 0)) {
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

  if(strstr(cmd, command[0])) {  //list
    res = mysql_store_result(&conn);
    while( (row = mysql_fetch_row(res)) != NULL) {
      sprintf(content, "%s%s  ", content, row[0]);
    }
    sprintf(content, "%s\n", content);
  }
  else if(strstr(cmd, command[1])) {  //info
    res = mysql_store_result(&conn);
    while( (row = mysql_fetch_row(res)) != NULL) {
      if(strstr(value, row[0])) {
        sprintf(content, "%s%d, %0.2f \n", content, atoi(row[2]), atof(row[3]));
        id = atoi(row[1]);
        strcpy(dbInfo, row[3]);
      }
    }
    sprintf(query, "select max(seqNum), avg(data), min(data), max(data) from sensorData%0.2d;", id);
    if(mysql_query(&conn, query)) {
      sprintf(content, "%s%s\n", content, mysql_error(&conn));
      exit(1); 
    }  
    res = mysql_store_result(&conn);
    while( (row = mysql_fetch_row(res)) != NULL) {
      sprintf(content, "%sid\tcount\tavg\tmin\tmax\tlast value\n", content);
      sprintf(content, "%s%d\t%s\t%0.5s\t%s\t%s\t%s\n", content, id, row[0], row[1], row[2], row[3], dbInfo);
    }
  }
  else if(strstr(cmd, command[2])) {  //get
    res = mysql_store_result(&conn);
    while( (row = mysql_fetch_row(res)) != NULL) {
      if(strstr(value, row[0])) {
       id = atoi(row[1]);
       idCount = atoi(row[2]);
     }
   }    
   sprintf(query, "select *from sensorData%0.2d order by seqnum desc;", id);
   if(mysql_query(&conn, query)) {
    sprintf(content, "%s%s\n", content, mysql_error(&conn));
    exit(1); 
  }  
  res = mysql_store_result(&conn);
  while( (row = mysql_fetch_row(res)) != NULL) {
    rowCount++;
    strcpy(dbTime[rowCount], row[0]);
    dbVal[rowCount] = atof(row[1]);
  }
  if(cnt == 2) {  //get name
    sprintf(content, "%s%s, %0.2f\n", content, dbTime[0], dbVal[0]);
  }
  else if(cnt == 3) {  //get name count
    if(idCount > realCount) {
      for(i = 0; i < realCount; i++)
        sprintf(content, "%s%s, %0.2f\n", content, dbTime[i], dbVal[i]);
    }
    else {
      for(i = 0; i < idCount; i++)
        sprintf(content, "%s%s, %0.2f\n", content, dbTime[i], dbVal[i]);
    }
  }
}

  mysql_free_result(res);
  mysql_close(&conn);
  sprintf(content, "%s(200 OK)\n", content);
  printf("Content-Length: %d\r\n", (int)strlen(content));
  printf("Content-Type: text/database\r\n\r\n");
  printf("%s", content);
  fflush(stdout);
}
void htmlReturn(void)
{
  char content[MAXLINE];
  char *buf;
  char *ptr;

  /* Make the response body */
  sprintf(content, "%s<html>\r\n<head>\r\n", content);
  sprintf(content, "%s<title>CGI test result</title>\r\n", content);
  sprintf(content, "%s</head>\r\n", content);
  sprintf(content, "%s<body>\r\n", content);
  sprintf(content, "%s<h2>Welcome to the CGI program</h2>\r\n", content);
  buf = getenv("QUERY_STRING");
  sprintf(content,"%s<p>Env : %s</p>\r\n", content, buf);
  ptr = strsep(&buf, "&");
  while (ptr != NULL){
    sprintf(content, "%s%s\r\n", content, ptr);
    ptr = strsep(&buf, "&");
  }
  sprintf(content, "%s</body>\r\n</html>\r\n", content);
  
  /* Generate the HTTP response */
  printf("Content-Length: %d\r\n", (int)strlen(content));
  printf("Content-Type: text/html\r\n\r\n");
  printf("%s", content);
  fflush(stdout);
}

void textReturn(void)
{
  char content[MAXLINE];
  char *buf;
  char *ptr;

  buf = getenv("QUERY_STRING");
  sprintf(content,"%sEnv : %s\n", content, buf);
  ptr = strsep(&buf, "&");
  while (ptr != NULL){
    sprintf(content, "%s%s\n", content, ptr);
    ptr = strsep(&buf, "&");
  }
  
  /* Generate the HTTP response */
  printf("Content-Length: %d\n", (int)strlen(content));
  printf("Content-Type: text/plain\r\n\r\n");
  printf("%s", content);
  fflush(stdout);
}

int main(void)
{
  databaseReturn();
  //htmlReturn();
  //textReturn();
  return(0);
}