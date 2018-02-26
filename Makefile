CC = gcc
CFLAG = -c
DEBUG = -g

ALL.O = util.o server.o client.o

all: $(ALL.O)
	# $(CC) main_server.c list.o DS_Struct.o utils.o server.o -o main_server.out
	# $(CC) data_server.c list.o DS_Struct.o utils.o server.o client_connect.o -o data_server.out
	# $(CC) client.c list.o DS_Struct.o utils.o client_connect.o -o client.out

util.o: util.c util.h
	$(CC) $(CFLAG) util.c

server.o: server.c util.h util.o
	$(CC) $(CFLAG) server.c

client.o: client.c client.h util.h util.o
	$(CC) $(CFLAG) client.c
clean:
	rm *.o *.out
