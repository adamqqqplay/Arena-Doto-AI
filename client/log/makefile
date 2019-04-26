all: demo.out

demo.out: makefile demo.o log.o 
ifeq ($(OS),Windows_NT)
	g++ demo.o log.o -o demo.out -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11
else
	g++ demo.o log.o -o demo.out -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11 -pthread
endif

demo.o: makefile demo.c 
	g++ -c demo.c -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11

log.o: makefile log.h log.cpp
	g++ -c log.cpp -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11

.PHONY: clean

clean:
ifeq ($(OS),Windows_NT)
	del -r *.out *.o
else
	rm -r *.out *.o
	rm -rf ./log
endif
