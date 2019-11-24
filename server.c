#include "stems.h"
#include "request.h"
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

pthread_t *threads;

typedef struct
{
	int port;
	int connfd;
	long time;
}BUFF;

BUFF *buffer_queue;

int MAX_queue = 0;
int front = 0, rear = 0;
sem_t mutex, empty, full;//  이 값이 0 이면 해당자원에 접근할수 없고,0 보다 크면 해당자원에 접근할수 있다.
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER; //
													// 
													// To run:
													// 1. Edit config-ws.txt with following contents
													//    <port number>
													// 2. Run by typing executable file
													//    ./server
													// Most of the work is done within routines written in request.c
													//
static int count = 0;


void Send(int connfd, int port, long time)
{
	sem_wait(&empty);
	sem_wait(&mutex);

	buffer_queue[rear].connfd = connfd;
	buffer_queue[rear].port = port;
	buffer_queue[rear].time = time;
	//printf("inqueue!!  %d\n",rear);
	rear = ++rear%MAX_queue;

	sem_post(&mutex);
	sem_post(&full);
}

BUFF Recv() { //버퍼(큐)로 부터 가져오기

	sem_wait(&full);
	sem_wait(&mutex);

	BUFF data = buffer_queue[front];
	//printf("dequeue!!  %d\n",front);
	front = ++front%MAX_queue;

	sem_post(&mutex);
	sem_post(&empty);

	return data;
}

void *worker(void *data)
{
	BUFF Data;
	Data = Recv();

	while (1) {
		consumer(Data.connfd, Data.time);
		Data = Recv();
	}

}

void getargs_sw(int *port, int *thread_pool_size, int *queue_size)
{
	FILE *fp;

	if ((fp = fopen("config-ws.txt", "r")) == NULL)
		unix_error("config-ws.txt file does not open.");

	fscanf(fp, "%d", port);
	fscanf(fp, "%d", thread_pool_size);
	fscanf(fp, "%d", queue_size);
	fclose(fp);
}


void consumer(int connfd, long arrivalTime)
{
	//printf("Consumer_connfd_num %d time %lu", connfd, arrivalTime);
	requestHandle(connfd, arrivalTime);
	Close(connfd);
}


int main(void)
{
	int listenfd, connfd, port, clientlen, thread_pool_size, queue_size;

	struct sockaddr_in clientaddr;
	pid_t pid;
	initWatch();
	//getargs_ws(&port);
	getargs_sw(&port, &thread_pool_size, &queue_size);

	MAX_queue = queue_size;

	sem_init(&mutex, 0, 1);
	sem_init(&full, 0, 0);
	sem_init(&empty, 0, queue_size);

	threads = (pthread_t*)malloc(sizeof(pthread_t)*thread_pool_size);
	buffer_queue = (BUFF*)malloc(sizeof(BUFF)*(queue_size));



	listenfd = Open_listenfd(port);

	for (int i = 0; i < thread_pool_size; i++) {
		pthread_create(&(threads[i]), NULL, &worker, NULL);
		pthread_detach(threads[i]);
	}


	while (1) {
		if (count == 0)
		{
			count = 1;
			pid = fork();
		}
		if (pid == 0)
			break;

		clientlen = sizeof(clientaddr);

		connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);

		Send(connfd, port, getWatch());
	}
	if (pid == 0)
		Execve("pushClient", NULL, environ);


	return(0);
}