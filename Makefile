SOURCES=main.c
OBJECTS=main.o
TARGET=pipepanic

# Settings for x86.
CC=gcc
CFLAGS=-O2 -Wall -pedantic $(shell sdl2-config --cflags)
LINK=gcc
LDFLAGS=
LIBS=$(shell sdl2-config --libs)
ifneq (,$(DEBUG))
	CFLAGS += -g -DDEBUG
endif


all:
	$(CC) $(CFLAGS) -c $(SOURCES)
	$(LINK) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(TARGET)

.PHONY: clean

clean:
	rm $(OBJECTS)
	rm $(TARGET)
