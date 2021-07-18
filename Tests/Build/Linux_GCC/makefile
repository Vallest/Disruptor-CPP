CPPFLAGS=-c -O2 -ansi -Wall -std=c++11 -I../../Source -I../../../Include

Tests: Tests.o Main.o UnitTesting.o
	@ mkdir -p ../../Bin/
	g++ -pthread -o ../../Bin/Tests Tests.o Main.o UnitTesting.o

Tests.o:
	g++ $(CPPFLAGS) ../../Source/Tests.cpp

Main.o:
	g++ $(CPPFLAGS) ../../Source/Main.cpp

UnitTesting.o:
	g++ $(CPPFLAGS) ../../Source/UnitTesting.cpp

.PHONY: clean

clean:
	rm -f *.o ../../Bin/Tests
