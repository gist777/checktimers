CC=gcc
CFLAGS=-g -Wall
LDLIBS=-lsqlite3

OBJ=checktimers.o error.o
EXE=checktimers
HDRS=checktimers.h error.h

all: $(EXE)

$(EXE): $(OBJ)

$(OBJ): $(HDRS)

.PHONY : all clean install tags
clean:
	-rm $(EXE) $(OBJ) addtimer vactimers
install:
	-cp checktimers addtimer
	-cp checktimers vactimers
tags:
	find . -type f -iname '*.[ch]' | xargs etags
