all: test
	
test: test.cpp
	g++ -std=c++98 -Wall -I../ -c test.cpp
	g++ -std=c++98 -Wall -o test -I../ test.o -L../ -latomlog

clean:
	rm -f *.o
	rm -f test
