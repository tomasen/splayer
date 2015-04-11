CC=gcc
 
CCFLAGS=-Wall -g

LDFLAGS=-fPIC -shared

BINPATH=../..

SRC = table.c

INCLUDE = -I./
 
all: init libtable.a
	cp -rf libtable.a  $(BINPATH)/lib
	cp -rf table.h $(BINPATH)/include
 
init:
	mkdir -p $(BINPATH)/include
	mkdir -p $(BINPATH)/lib

libtable.a: table.o
	ar r libtable.a table.o

table.o: table.h table_loc.h TableConfig.h
	$(CC) -c $(CCFLAGS) table.c

clean:
	rm -rf $(BINPATH)/include
	rm -rf $(BINPATH)/lib
	rm -rf libtable.a
	rm -rf table.o
