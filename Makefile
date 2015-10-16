CC=gcc
C_FLAGS=$(CFLAGS) -Wall -O2 -lglpk -lm -g -lrt
SOURCES=main.c matrix.c utils.c list.c
BIN=main

.PHONY: clean

all:
	$(CC) $(SOURCES) -o $(BIN) $(C_FLAGS) 

answer:
	$(CC) $(SOURCES) -o $(BIN) $(C_FLAGS) -D__output_answer

obj:
	$(CC) $(SOURCES) -o $(BIN) $(C_FLAGS) -D__output_obj

progress:
	$(CC) $(SOURCES) -o $(BIN) $(C_FLAGS) -D__output_progress

clean:
	-@rm -rf core $(BIN) 2> /dev/null || true
