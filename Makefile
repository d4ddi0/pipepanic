SOURCES=main.c
OBJECTS=main.o
TARGET=pipepanic

# Settings for x86.
CC=gcc
CFLAGS=-O2 -Wall -pedantic `sdl-config --cflags`
LINK=gcc
LDFLAGS=
LIBS=`sdl-config --libs`

all:
	$(CC) $(CFLAGS) -c $(SOURCES)
	$(LINK) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(TARGET)

.PHONY: clean

clean:
	rm $(OBJECTS)
	rm $(TARGET)
