# 046267 Computer Architecture - Spring 2020 - HW #1
# makefile for test environment

all: bp_main

# Environment for C 
CFLAGS = -std=c99 -Wall

# Environment for C++ 
CPP = g++
CCFLAGS = -std=c++11 -Wall

# Automatically detect whether the bp is C or C++
# Must have either bp.c or bp.cpp - NOT both
SRC_FILES := bp_main.c bp_wrapper.c
OBJS := $(patsubst %.c,%.o,$(SRC_FILES)) 




bp_main: $(OBJS) bp.o
	$(CPP) $^ -o $@  

$(OBJS): %.o: src/%.c
	$(CPP) -c $(CCFLAGS) $^

bp.o: src/bp.cpp
	$(CPP) -c $(CCFLAGS) $^
.PHONY: clean
clean:
	rm -f bp_main $(OBJS) bp.o
