all: socket sockserver
	
socket: socket.cpp
	g++ -std=c++0x -Wall -I../ -c socket.cpp
	g++ -std=c++0x -Wall -o socket -I../ socket.o -L../ -latomsocket

sockserver: sockserver.cpp
	g++ -std=c++0x -Wall -I../ -c sockserver.cpp
	g++ -std=c++0x -Wall -o sockserver -I../ sockserver.o -L../ -latomsocket

clean:
	rm -f *.o
	rm -f socket
	rm -f sockserver
