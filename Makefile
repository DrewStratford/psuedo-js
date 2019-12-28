VPATH = src include build

CFLAGS ?= -g -std=c++17 -Wl,--export-dynamic
NAME ?= Language



CC       := gcc
CXX     := g++
LIBS     := -lm -ldl
INCLUDES := -Iinclude
CFLAGS   := $(CFLAGS) $(INCLUDES) $(LIBS)
BUILDDIR := build

OBJS :=\
main.o\
object.o\
context.o\
instruction.o\
ast.o\

PARSOBJS :=\
object.o\
context.o\
instruction.o\
ast.o\
parser.o

OBJECTOBJS :=\
object.o\

all : lang

parser : $(PARSOBJS)
	$(CXX) -o parser  $(CFLAGS) $(PARSOBJS:%.o=build/%.o)

obj : $(OBJECTOBJS)
	$(CXX) -o obj -D OBJECT_MAIN $(CFLAGS) $(OBJECTOBJS:%.o=build/%.o)
	

lang : $(OBJS)
	$(CXX) -o $@  $(CFLAGS) $(OBJS:%.o=build/%.o)

%.o : %.cpp
	$(CXX) -o $(BUILDDIR)/$@ -c $(CFLAGS) $<

%.o : %.c
	$(CC) -o $(BUILDDIR)/$@ -c $(CFLAGS) $<

.PHONY: clean all
clean:
	rm -f lang
	rm -f build/*
