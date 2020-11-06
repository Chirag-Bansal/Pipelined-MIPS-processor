all:
	g++ -c pscr.cpp
	g++ pscr.o -o p

run:
	./p

clean:
	rm -rf *.o p