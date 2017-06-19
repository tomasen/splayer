## This is the makefile for sinet 
## Soleo Shao 2011.3.2
## The libarary is static. If you wanna have a shared library, you should modify the makefile.
TOP = .
TCC = g++
AR = ar cr
RANLIB = ranlib
MKSHLIB = g++ 
LD = ld
LIB = libsinet.a

all:
	echo "Compiling sphash static library\n";\
	$(TCC) -c $(TOP)/sinet/*.cpp $(TOP)/sinet/*.cc;\
	$(AR) $(LIB) $(TOP)/*.o;\
	$(RANLIB) $(LIB);

clean:
	rm -rf *.o $(LIB)
