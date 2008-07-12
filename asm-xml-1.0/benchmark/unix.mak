# Build benchmarks

ASMXML_INC = ../include
ASMXML_OBJ = ../obj/elf/asm-xml.o

CC = g++
CFLAGS = -O2 -I $(ASMXML_INC)
#LFLAGS = -s -lexpat -lxerces-c_2
LFLAGS = -s -lexpat

all: bench

bench: bench.o $(ASMXML_OBJ)
	$(CC) $(LFLAGS) $^ -o $@

bench.o: bench.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm *.o bench

