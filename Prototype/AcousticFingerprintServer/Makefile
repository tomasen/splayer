CC = gcc
INCDIR = -I./include -I./libs
LDFLAGS = ./lib/libphashaudio.a

TARGET = afserverd

all:
	$(CC) AFServer.c $(LDFLAGS) -o $(TARGET) $(INCDIR) -lzmq
clean:
	rm -rf *.o $(TARGET)
