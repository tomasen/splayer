CC=gcc
 
CCFLAGS=-Wall -g -O3 -fno-exceptions -fno-rtti

LDFLAGS=-fPIC -shared 

BINPATH=.

INCLUDE = -I./
 
all: init libphashaudio.a
	cp -rf libphashaudio.a  $(BINPATH)/lib
	cp -rf audiophash.h $(BINPATH)/include
 
init:
	mkdir -p $(BINPATH)/include
	mkdir -p $(BINPATH)/lib

libphashaudio.a: ph_fft.o  phash.o audiophash.o
	ar crv libphashaudio.a  ph_fft.o phash.o audiophash.o;\
	ranlib libphashaudio.a
audiophash.o: pHash-config.h audiophash.h
	$(CC) -c $(CCFLAGS) audiophash.cpp
phash.o: pHash.h
	$(CC) -c $(CCFLAGS) pHash.cpp
ph_fft.o: ph_fft.h
	$(CC) -c $(CCFLAGS) ph_fft.c
clean:
	rm -rf $(BINPATH)/include
	rm -rf $(BINPATH)/lib
	rm -rf libphashaudio.a
	rm -rf *.o
