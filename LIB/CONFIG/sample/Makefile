all: test
	
test: test.cpp
	g++ -std=c++98 -Wall -I../ -I../../LOG -c test.cpp
	g++ -std=c++98 -Wall -o test test.o -L../ -L../../LOG -latomconfig -latomlog -lrt

clean:
	rm -f *.o
	rm -f test
