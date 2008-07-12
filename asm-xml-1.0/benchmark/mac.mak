# Build benchmarks

ASMXML_INC = ../include
ASMXML_OBJ = ../obj/mach-o/asm-xml.o

CC = g++
#CFLAGS = -O2 -I $(ASMXML_INC)
CFLAGS = -g -I $(ASMXML_INC) -I /sw/include
#LFLAGS = -s -lexpat -lxerces-c_2
LFLAGS = -L/sw/lib -lexpat 

all: bench

bench: bench.o $(ASMXML_OBJ)
	$(CC) $(LFLAGS) $^ -o $@

bench.o: bench.cpp
	$(CC) -DMACOS $(CFLAGS) -c $^ -o $@

clean:
	rm *.o bench

