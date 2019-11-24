#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <semaphore.h>
#include <sys/time.h>
#include <math.h>
#include <wiringPi.h>
#include "stems.h"
#include <time.h>
#define MAXTIMINGS 83
#define DHTPIN 7
struct timeval startTime;
int dht11_dat[5] = {0, }; // 모두다 0으로 초기화

int read_dht11_dat()
{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;
	uint8_t flag = HIGH;
	uint8_t state = 0;
	
	dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;
	pinMode(DHTPIN, OUTPUT);
	digitalWrite(DHTPIN, LOW);
	delay(18);
	digitalWrite(DHTPIN, HIGH);
	delayMicroseconds(30);
	pinMode(DHTPIN, INPUT);
	for(i = 0; i < MAXTIMINGS; i++){
		counter = 0;
		while(digitalRead(DHTPIN) == laststate){
			counter++;
			delayMicroseconds(1);
			if(counter == 200) break;
		}
 
		laststate = digitalRead(DHTPIN);
 
		if(counter == 200) break;
		if((i >= 4) && (i%2 == 0)){
			dht11_dat[j / 8] <<= 1;
			if(counter > 20)
				dht11_dat[j / 8] |= 1;
			j++;
		}
	}
	if((j >= 40) && (dht11_dat[4] == ((dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xff))){
 
		printf("humidity = %d.%d %% Temperature = %d.%d *C\n", dht11_dat[0], dht11_dat[1], dht11_dat[2], dht11_dat[3]);
	}
	else {
		printf("Data get failed\n");
		return 1;
	}
 
	return 2;
 
}
 
 
void clientSend(int fd, char *filename, char *body)
{
	char buf[MAXLINE];
	char hostname[MAXLINE];
	Gethostname(hostname, MAXLINE);
 	
	sprintf(buf, "POST %s HTTP/1.1\n", filename);
	sprintf(buf, "%sHost: %s\n", buf, hostname);
	sprintf(buf, "%sContent-Type: text/plain; charset=utf-8\n", buf);
	sprintf(buf, "%sContent-Length: %d\n\r\n", buf, strlen(body));
	sprintf(buf, "%s%s\n", buf, body);
	Rio_writen(fd, buf, strlen(buf));
 
}
 
void clientPrint(int fd)
{
	rio_t rio;
	char buf[MAXBUF];
	int n;
	int length=0;
	Rio_readinitb(&rio, fd);
 
 
	n = Rio_readlineb(&rio, buf, MAXBUF);
	while (strcmp(buf, "\r\n") && (n > 0)) {
		if (sscanf(buf, "Content-Length: %d ", &length) == 1)
			printf("Length = %d\n", length);
 
		printf("Header: %s", buf);
		n = Rio_readlineb(&rio, buf, MAXBUF);
 
	}
 
 
	n = Rio_readlineb(&rio, buf, MAXBUF);
	while (n > 0) {
		printf("%s", buf);
		n = Rio_readlineb(&rio, buf, MAXBUF);
 
	}
 
}

 
void userTask(char *hostname, int port, char *filename, char *time)
{
	int clientfd, i;
	char msg[MAXLINE];
 	

	for(i = 0; i < 2; i++){
	
		if(i == 0)
			sprintf(msg, "name=temperature&time=%s&value=%d.%d", time, dht11_dat[2], dht11_dat[3]);
		else if(i == 1)
			sprintf(msg, "name=humidity&time=%s&value=%d.%d", time, dht11_dat[0], dht11_dat[1]);
		

		clientfd = Open_clientfd(hostname, port);
		clientSend(clientfd, filename, msg);
		clientPrint(clientfd);
		Close(clientfd);
	}
 
}
 
 
 
void getargs_cp(char *hostname, int *port, char *filename, float *times)
{
	FILE *fp;
	fp = fopen("config-pi.txt", "r");
	if (fp == NULL)
		unix_error("config-pi.txt file does not open.");
 
	fscanf(fp, "%s", hostname);
	fscanf(fp, "%d", port);
	fscanf(fp, "%s", filename);
	fscanf(fp, "%f", times);
	fclose(fp);
 
}
 
 
 
int main(void)
{
	char hostname[MAXLINE], filename[MAXLINE];
	int port, check;
	float times;
	char *chtime=NULL;
	time_t now;
	time(&now);
	chtime=ctime(&now);

	getargs_cp(hostname, &port, filename, &times);

	if(wiringPiSetup() == -1)
		exit (1);
 
	while (1){
		sleep(times);
		check = read_dht11_dat();
		while(check == 1){
			delay(1000);
			check = read_dht11_dat();
 
		}
		chtime=ctime(&now);
		chtime[strlen(chtime)-1]='\0';
		userTask(hostname, port, filename, chtime);
 
	}
 
	return(0);
 
}
