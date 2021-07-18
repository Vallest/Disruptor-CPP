CPPFLAGS=-c -O2 -ansi -Wall -std=c++11 -I../../Source -I../../../Include

Tests: Tests.o Main.o UnitTesting.o
	@ mkdir -p ../../Bin/
	clang -lstdc++ -pthread -lm -o ../../Bin/Tests Tests.o Main.o UnitTesting.o

Tests.o:
	clang $(CPPFLAGS) ../../Source/Tests.cpp

Main.o:
	clang $(CPPFLAGS) ../../Source/Main.cpp

UnitTesting.o:
	clang $(CPPFLAGS) ../../Source/UnitTesting.cpp

.PHONY: clean

clean:
	rm -f *.o ../../Bin/Tests
