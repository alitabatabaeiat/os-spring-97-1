CC = gcc
CFLAG = -c
DEBUG = -g

ALL.O = util.o server.o client.o

all: $(ALL.O)
	$(CC) util.o server.o -o s.o
	$(CC) util.o client.o -o c.o

util.o: util.c util.h
	$(CC) $(CFLAG) util.c

server.o: server.c util.h util.o
	$(CC) $(CFLAG) server.c

client.o: client.c util.h util.o
	$(CC) $(CFLAG) client.c
clean:
	rm *.o *.out
