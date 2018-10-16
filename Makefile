OBJECTS=pipepanic.o
TARGET=pipepanic

VERSION := $(shell git describe)
ifeq (,$(VERSION))
   VERSION := $(shell cat VERSION)
else
  $(shell echo $(VERSION) > VERSION)
endif

CC=gcc
CFLAGS=-O2 -Wall -pedantic -DVERSION=\"$(VERSION)\" \
	   $(shell sdl2-config --cflags)
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
