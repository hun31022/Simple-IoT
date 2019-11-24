#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/time.h>
#include "stems.h"

void clientSend(int fd, char *filename, char *body)
{
	char buf[MAXLINE];
	char hostname[MAXLINE];

	Gethostname(hostname, MAXLINE);

	/* Form and send the HTTP request */
	sprintf(buf, "POST %s HTTP/1.1\n", filename);
	sprintf(buf, "%sHost: %s\n", buf, hostname);
	sprintf(buf, "%sContent-Type: text/plain; charset=utf-8\n", buf);
	sprintf(buf, "%sContent-Length: %d\n\r\n", buf, (int)strlen(body));
	sprintf(buf, "%s%s\n", buf, body);
	Rio_writen(fd, buf, strlen(buf));
}

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
		n = Rio_readlineb(&rio, buf, MAXBUF);

		/* If you want to look for certain HTTP tags... */
		if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
		}
	}

	/* Read and display the HTTP Body */
	n = Rio_readlineb(&rio, buf, MAXBUF);
	while (n > 0) {
		printf("%s", buf);
		n = Rio_readlineb(&rio, buf, MAXBUF);
	}
}

void userTask(char *hostname, int port, char *filename, char *body)
{
	int clientfd;
	clientfd = Open_clientfd(hostname, port);
	clientSend(clientfd, filename, body);
	clientPrint(clientfd);
	Close(clientfd);
}

int main(void)
{
	char body[MAXLINE], body2[MAXLINE];
	char name[MAXLINE], time[MAXLINE], value[MAXLINE], temp[MAXLINE], 
								temp2[MAXLINE], temp3[MAXLINE];
	char hostname[MAXLINE], filename[MAXLINE], port[MAXLINE], threshold[MAXLINE];
	char *token;
	FILE *fp;
	int fd, bodyLen;
	int count = 0;
	float dvalue, dlimit, p_port;

	while (1){
		if((fd = open( "MYFIFO", O_RDWR)) == -1) {
    			//printf("fail to call open()\n");
  		}
		
		fp = fopen("config-pc.txt", "r+");
		if (fp == NULL)
			printf("config-pc error\n");

		read(fd,body,MAXLINE);
		bodyLen = atoi(body);
		if(bodyLen > 0)
			read(fd,body,MAXLINE);
		body[strlen(body)-1]='\0';

		fscanf(fp, "%s", hostname);
		fscanf(fp, "%s", port);
		fscanf(fp, "%s", filename);
		fscanf(fp, "%s", threshold);
		dlimit = atof(threshold);
		 
		strcpy(body2, body);
		token = strtok(body, "&");
		count = 0;
		while (token != NULL)
		{
			if (count == 0)
				strcpy(temp, token);
			if (count == 1)
				strcpy(temp2, token);
			if (count == 2)
				strcpy(temp3, token);
			token = strtok(NULL, "&");
			count++;
		}

		token = strtok(temp, "=");
		count = 0;
		while (token != NULL)
		{
			if (count == 1)
				strcpy(name, token);
			token = strtok(NULL, "=");
			count++;
		}
		token = strtok(temp2, "=");
		count = 0;
		while (token != NULL)
		{
			if (count == 1)
				strcpy(time, token);
			token = strtok(NULL, "=");
			count++;
		}
		token = strtok(temp3, "=");
		count = 0;
		while (token != NULL)
		{
			if (count == 1)
				strcpy(value, token);
			token = strtok(NULL, "=");
			count++;
		}
		p_port = atoi(port);
		dvalue = atof(value);
		
		if (dvalue > dlimit){
			userTask(hostname, p_port, filename, body2);
		}

		close(fd);
		fclose(fp);
		//unlink("MYFIFO");
	}
	return 0;
}