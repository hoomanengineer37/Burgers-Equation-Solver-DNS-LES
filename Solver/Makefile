CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS= -std=c++11
#LDFLAGS=-g $(shell root-config --ldflags)
#LDLIBS=$(shell root-config --libs)

all: a.out 

a.out: MainProgram.cpp Filter.cpp NUMERICALDERIVATION.h
	$(CXX) $(CPPFLAGS) MainProgram.cpp Filter.cpp
 
clean:
	$(RM) a.out
	
distclean: clean
	$(RM) a.out