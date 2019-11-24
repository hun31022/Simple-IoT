/*
* clientPost.c: A very, very primitive HTTP client for sensor
*
* To run, prepare config-cp.txt and try:
*      ./clientPost
*
* Sends one HTTP request to the specified HTTP server.
* Get the HTTP response.
*/


#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/time.h>
#include "stems.h"
#include <time.h>

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
		/* If you want to look for certain HTTP tags... */
		if (sscanf(buf, "Content-Length: %d ", &length) == 1)
			printf("Length = %d\n", length);
		printf("Header: %s", buf);
		n = Rio_readlineb(&rio, buf, MAXBUF);
	}

	/* Read and display the HTTP Body */
	n = Rio_readlineb(&rio, buf, MAXBUF);
	while (n > 0) {
		printf("%s", buf);
		n = Rio_readlineb(&rio, buf, MAXBUF);
	}
}

/* currently, there is no loop. I will add loop later */
void userTask(char *myname, char *hostname, int port, char *filename, char *time, float value)
{
	int clientfd;
	char msg[MAXLINE];

	sprintf(msg, "name=%s&time=%s&value=%f", myname, time, value);
	clientfd = Open_clientfd(hostname, port);
	clientSend(clientfd, filename, msg);
	clientPrint(clientfd);
	Close(clientfd);
}

void getargs_cp(char *myname, char *hostname, int *port, char *filename, float *value)
{
	FILE *fp;

	fp = fopen("config-cp.txt", "r");
	if (fp == NULL)
		unix_error("config-cp.txt file does not open.");

	fscanf(fp, "%s", hostname);
	fscanf(fp, "%d", port);
	fscanf(fp, "%s", filename);
	fscanf(fp, "%s", myname);
	fscanf(fp, "%f", value);
	fclose(fp);
}

void printMenu(void);
void printHelp(void);
char *operateName(char *, char *);
float operateValue(char *, float);
int main(void)
{
	char myname[MAXLINE], hostname[MAXLINE], filename[MAXLINE], savename[MAXLINE];

	int port;
	float value, chvalue, savevalue;

	char *command[5] = { "help", "name", "value", "send", "quit" };
	char inputC[MAXLINE];
	char *chname;

	char *chtime = NULL;

	time_t now;
	time(&now);

	chtime = ctime(&now);
	chtime[strlen(chtime) - 1] = '\0'; 

	getargs_cp(myname, hostname, &port, filename, &value);
	userTask(myname, hostname, port, filename, chtime, value);
	strcpy(savename, myname);
	savevalue = value;
	do {
		do {
			printMenu();
			printf(">> ");
			fgets(inputC, sizeof(inputC), stdin);
		} while (!strstr(inputC, command[0]) &&
			!strstr(inputC, command[1]) &&
			!strstr(inputC, command[2]) &&
			!strstr(inputC, command[3]) &&
			!strstr(inputC, command[4]));
		if (strstr(inputC, command[0]))  //help
			printHelp();
		else if (strstr(inputC, command[1])) { //name
			chname = operateName(inputC, savename);
			if (chname != NULL) {
				strcpy(savename, chname);
			}
		}
		else if (strstr(inputC, command[2])) {  //value
			chvalue = operateValue(inputC, savevalue);
			if (chvalue != 0) {
				savevalue = chvalue;
			}
		}
		else if (strstr(inputC, command[3])) {  //send
			time(&now);
			chtime = ctime(&now);
			chtime[strlen(chtime) - 1] = '\0';
			printf("Sending...name=%s&time=%s&value=%f\n", savename, chtime, savevalue);
			userTask(savename, hostname, port, filename, chtime, savevalue);
		}

	} while (!strstr(inputC, command[4]));

	return(0);
}
void printMenu(void)
{
	printf("-------------------------------------------------\n");
	printf("If you want to see commands, type 'help' \n");
	printf("You must input commands correctly! \n");
	printf("-------------------------------------------------\n");
}

void printHelp(void)
{
	printf(" help : list available commands. \n");
	printf(" name : print current sensor name. \n");
	printf(" name <sensor> : change sensor name to <sensor>. \n");
	printf(" value : print current value of sensor. \n");
	printf(" value <n> : set sensor value to <n>. \n");
	printf(" send : send (current sensor name, time, value) to server. \n");
	printf(" quit : quit the program. \n");
}

char *operateName(char *command, char *currentName)
{
	char *tmp, *tokNameTmp, *sendName;
	char changeName[100], tokName[100], space[2] = " ";
	char *setnull = NULL;
	int count = 0;
	command[strlen(command) - 1] = '\0'; 

	if (!strstr(command, " ")) {
		printf("Current sensor is '%s' \n", currentName);
		return setnull;
	}
	else {
		tmp = strtok(command, " ");
		while (tokNameTmp = strtok(NULL, " ")) {
			++count;
			strcpy(tokName, tokNameTmp);
			if (count > 1) {
				strcat(changeName, space);
				strcat(changeName, tokName);
			}
			else
				strcpy(changeName, tokName);
		}
		//printf("%s\n", changeName);
		sendName = changeName;
		return sendName;
	}
}

float operateValue(char *command, float currentValue)
{
	float changeValue;
	char *sepF, *tmp;
	command[strlen(command) - 1] = '\0'; 

	if (!strstr(command, " ")) {
		printf("Current value of sensor is '%f' \n", currentValue);
		return 0;
	}
	else {
		tmp = strtok(command, " ");
		sepF = strtok(NULL, " ");
		changeValue = atof(sepF);
		return changeValue;
	}
}

