# Build Examples - MinGW

ASMXML_INC = ../include
ASMXML_OBJ = ../obj/ms-coff/asm-xml.obj

CC = gcc
CFLAGS = -O2 -fomit-frame-pointer -I $(ASMXML_INC)
LFLAGS = -s

all: simple xml-to-table db2html

simple: simple.o $(ASMXML_OBJ)
	$(CC) $(LFLAGS) $^ -o $@

simple.o: c/simple/simple.c
	$(CC) $(CFLAGS) -c $^ -o $@

xml-to-table: xml-to-table.o $(ASMXML_OBJ)
	$(CC) $(LFLAGS) $^ -o $@

xml-to-table.o: c/xml-to-table/xml-to-table.c
	$(CC) $(CFLAGS) -c $^ -o $@

db2html: docbook-to-html.o $(ASMXML_OBJ)
	$(CC) $(LFLAGS) $^ -o $@

docbook-to-html.o: cpp/docbook-to-html/docbook-to-html.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	del  *.o table.csv simple.exe xml-to-table.exe db2html.exe

