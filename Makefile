CC=gcc
C_FLAGS=$(CFLAGS) -Wall -O2 -lglpk -lm -g -lrt
SOURCES=main.c matrix.c utils.c mpi_bnb.c
BIN=main

# mpich
#MPISTUFF=-D_FORTIFY_SOURCE=2 -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -Wl,-Bsymbolic-functions -Wl,-z,relro -I/usr/include/mpich -L/usr/lib/x86_64-linux-gnu -lmpich -lopa -lmpl -lrt -lcr -lpthread

# Openmpi
MPISTUFF=-I/usr/lib/openmpi/include -I/usr/lib/openmpi/include/openmpi -pthread -L/usr//lib -L/usr/lib/openmpi/lib -lmpi -ldl -lhwloc

.PHONY: clean

all:
	$(CC) $(SOURCES) -o $(BIN) $(MPISTUFF) $(C_FLAGS) 
                                               
answer:                                        
	$(CC) $(SOURCES) -o $(BIN) $(MPISTUFF) $(C_FLAGS) -D__output_answer
                                               
obj:                                           
	$(CC) $(SOURCES) -o $(BIN) $(MPISTUFF) $(C_FLAGS) -D__output_obj
                                               
progress:                                      
	$(CC) $(SOURCES) -o $(BIN) $(MPISTUFF) $(C_FLAGS) -D__output_progress

clean:
	-@rm -rf core $(BIN) 2> /dev/null || true

