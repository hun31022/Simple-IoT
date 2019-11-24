#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = server.o request.o stems.o clientGet.o clientPost.o pushClient.o pushServer.o
TARGET = server

CC = gcc
CFLAGS = -g -Wall

LIBS = -lpthread 

.SUFFIXES: .c .o 

all: server clientPost clientGet dataGet.cgi dataPost.cgi pushClient pushServer alarm.cgi

server: server.o request.o stems.o
	$(CC) $(CFLAGS) -o server server.o request.o stems.o $(LIBS)

clientGet: clientGet.o stems.o
	$(CC) $(CFLAGS) -o clientGet clientGet.o stems.o

clientPost: clientPost.o stems.o
	$(CC) $(CFLAGS) -o clientPost clientPost.o stems.o $(LIBS)

dataGet.cgi: dataGet.c stems.o
	$(CC) $(CFLAGS) -o dataGet.cgi dataGet.c stems.o -I/usr/local/include/mysql -L/usr/local/lib/mysql -lmysqlclient

dataPost.cgi: dataPost.c stems.o
	$(CC) $(CFLAGS) -o dataPost.cgi dataPost.c stems.o -I/usr/local/include/mysql -L/usr/local/lib/mysql -lmysqlclient

pushClient: pushClient.o stems.o
	$(CC) $(CFLAGS) -o pushClient pushClient.o request.o stems.o $(LIBS)

pushServer: pushServer.o request.o stems.o
	$(CC) $(CFLAGS) -o pushServer pushServer.o request.o stems.o $(LIBS)

alarm.cgi: alarm.c stems.o
	$(CC) $(CFLAGS) -o alarm.cgi alarm.c stems.o request.o


.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

server.o: stems.h request.h
clientGet.o: stems.h
clientPost.o: stems.h

clean:
	-rm -f $(OBJS) server clientPost clientGet dataGet.cgi dataPost.cgi pushClient pushServer alarm.cgi
