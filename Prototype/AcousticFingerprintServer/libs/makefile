CC=gcc
 
CCFLAGS=-Wall -g -O3

LDFLAGS=-fPIC -shared

BINPATH=..

INCLUDE = -I./
 
all: init libphashaudio.a
	cp -rf libphashaudio.a  $(BINPATH)/lib
	cp -rf phash_audio.h $(BINPATH)/include
 
init:
	mkdir -p $(BINPATH)/include
	mkdir -p $(BINPATH)/lib
	cp table-4.3.0phmodified/table.o ./table.o

libphashaudio.a: complex.o fft.o phash_audio.o
	ar crv libphashaudio.a complex.o fft.o phash_audio.o table.o;\
	ranlib libphashaudio.a

phash_audio.o: pHashAudioConfig.h phash_audio.h
	$(CC) -c $(CCFLAGS) phash_audio.c
fft.o: fft.h
	$(CC) -c $(CCFLAGS) fft.c
complex.o: complex.h
	$(CC) -c $(CCFLAGS) complex.c
clean:
	rm -rf $(BINPATH)/include
	rm -rf $(BINPATH)/lib
	rm -rf libphashaudio.a
	rm -rf *.o
