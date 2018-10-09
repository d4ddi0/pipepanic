OBJECTS=pipepanic.o
TARGET=pipepanic

CC=gcc
CFLAGS=-O2 -Wall -pedantic $(shell sdl2-config --cflags)
LDFLAGS=
LDLIBS=$(shell sdl2-config --libs)
ifneq (,$(DEBUG))
	CFLAGS += -g -DDEBUG
endif

all: pipepanic

pipepanic: pipepanic.o

.PHONY: clean

clean:
	rm $(OBJECTS)
	rm $(TARGET)
