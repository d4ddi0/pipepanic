# Settings for x86.
CC=gcc
CFLAGS=-O2 -Wall -pedantic $(shell sdl2-config --cflags)
LINK=gcc
LDFLAGS=
LDLIBS=$(shell sdl2-config --libs)
ifneq (,$(DEBUG))
	CFLAGS += -g -DDEBUG
endif


all: pipepanic


pipepanic: main.o
	$(LINK) $(LDFLAGS) $< $(LDLIBS) -o $@

.PHONY: clean

clean:
	rm $(OBJECTS)
	rm $(TARGET)
