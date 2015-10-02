CC=gcc
MPISTUFF=-D_FORTIFY_SOURCE=2 -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -Wl,-Bsymbolic-functions -Wl,-z,relro -I/usr/include/mpich -L/usr/lib/x86_64-linux-gnu -lmpich -lopa -lmpl -lrt -lcr -lpthread
CFLAGS=-Wall -O2 -lglpk -lm -g
SOURCES=main.c matrix.c utils.c mpi_bnb.c
BIN=main

.PHONY: clean

all:
	$(CC) $(SOURCES) -o $(BIN) $(MPISTUFF) $(CFLAGS) 
                                               
answer:                                        
	$(CC) $(SOURCES) -o $(BIN) $(MPISTUFF) $(CFLAGS) -D__output_answer
                                               
obj:                                           
	$(CC) $(SOURCES) -o $(BIN) $(MPISTUFF) $(CFLAGS) -D__output_obj
                                               
progress:                                      
	$(CC) $(SOURCES) -o $(BIN) $(MPISTUFF) $(CFLAGS) -D__output_progress

clean:
	-@rm -rf core $(BIN) 2> /dev/null || true
