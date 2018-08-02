CC = gcc
CFLAGS = -std=gnu99

ftclient: ftclient.o
	$(CC) $(CFLAGS) -o ftclient ftclient.o

ftclient.o: ftclient.h

clean:
	rm *.o
	rm -f ftclient