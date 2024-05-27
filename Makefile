
CC = gcc
CFLAGS= -Wall -g -pedantic -std=c99
LDFLAGS= -lz


all: sender receiver

sender: sender.o utils.o sha256.o
	$(CC) $(CFLAGS) -o sender sender.o utils.o sha256.o $(LDFLAGS)

receiver: receiver.o utils.o sha256.o
	$(CC) $(CFLAGS) -o receiver receiver.o utils.o sha256.o $(LDFLAGS)

sender.o: sender.c
	$(CC) $(CFLAGS) -c sender.c

receiver.o: receiver.c
	$(CC) $(CFLAGS) -c receiver.c

utils.o: utils.c 
	$(CC) $(CFLAGS) -c utils.c

sha256.o: sha256.c
	$(CC) $(CFLAGS) -c sha256.c

clean:
	rm -f sender receiver sender.o receiver.o utils.o sha256.o OUTPUT1.*
	clear
