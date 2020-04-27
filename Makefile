TARGETS=server

CFLAGS=-Wall -g -O0

all: $(TARGETS)

homework5: server.c
	gcc $(CFLAGS) -o server server.c -lpthread

clean:
	rm -f $(TARGETS)

