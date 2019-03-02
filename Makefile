VPATH = src include build

CFLAGS ?= -g
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

all : lang

parser : $(PARSOBJS)
	$(CXX) -o parser  $(CFLAGS) $(PARSOBJS:%.o=build/%.o)
	

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
