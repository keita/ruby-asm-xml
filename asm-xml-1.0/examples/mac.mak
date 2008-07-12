# Build Examples

ASMXML_INC = ../include
ASMXML_OBJ = ../obj/mach-o/asm-xml.o

CC = gcc
CFLAGS = -O2  -I $(ASMXML_INC)
LFLAGS = 

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
	g++ $(LFLAGS) $^ -o $@

docbook-to-html.o: cpp/docbook-to-html/docbook-to-html.cpp
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f *.o table.csv simple xml-to-table db2html

