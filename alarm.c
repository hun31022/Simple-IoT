#include "stems.h"
#include <sys/time.h>
#include <assert.h>
#include <unistd.h>

int main()
{
	char *token;
	char name[MAXLINE], time[MAXLINE], value[MAXLINE], tempName[MAXLINE], tempTime[MAXLINE], tempValue[MAXLINE], bodyForTok[MAXLINE];
	char *takeBody;
	int count = 0;
	char buff[MAXLINE];

	takeBody = getenv("REQUEST_BODY");
	takeBody[strlen(takeBody)-1] = '\0';

	//Body Seperate
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
			strcpy(value, token);
		token = strtok(NULL, "=");
		count++;
	}
	value[strlen(value)-1] = '\0';

	fprintf(stderr, "경고 : %s sensor로부터 %s 시각에 %s 라는 값이 발생했습니다.\nsuccess WARNING!(200)\n", name, time, value);
	
	return 0;
}