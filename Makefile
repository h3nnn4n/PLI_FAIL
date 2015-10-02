CC=gcc
CFLAGS=-Wall -O2 -lglpk -lm -g
SOURCES=main.c matrix.c utils.c
BIN=main

.PHONY: clean

all:
	$(CC) $(SOURCES) -o $(BIN) $(CFLAGS) 

answer:
	$(CC) $(SOURCES) -o $(BIN) $(CFLAGS) -D__output_answer

obj:
	$(CC) $(SOURCES) -o $(BIN) $(CFLAGS) -D__output_obj

progress:
	$(CC) $(SOURCES) -o $(BIN) $(CFLAGS) -D__output_progress

clean:
	-@rm -rf core $(BIN) 2> /dev/null || true
