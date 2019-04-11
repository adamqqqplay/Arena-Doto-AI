all: main.out

main.out: makefile main.o logic.h logic.o const.h playerAI.h playerAI.cpp jsoncpp/json/json-forwards.h jsoncpp/json/json.h jsoncpp/jsoncpp.cpp geometry.o geometry.h
ifeq ($(OS),Windows_NT)
	g++ main.o logic.o playerAI.cpp jsoncpp/jsoncpp.cpp geometry.o -o main.out -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -lwsock32 -std=c++11
else
	g++ main.o logic.o playerAI.cpp jsoncpp/jsoncpp.cpp geometry.o -o main.out -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11 -pthread
endif

logic.o: makefile const.h playerAI.h geometry.h logic.h jsoncpp/json/json-forwards.h jsoncpp/json/json.h logic.cpp 
	g++ -c logic.cpp -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11

geometry.o: makefile const.h playerAI.h geometry.h logic.h jsoncpp/json/json-forwards.h jsoncpp/json/json.h geometry.cpp
	g++ -c geometry.cpp -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11

main.o: makefile const.h playerAI.h geometry.h logic.h jsoncpp/json/json-forwards.h jsoncpp/json/json.h main.cpp
	g++ -c main.cpp -D_GLIBCXX_USE_CXX11_ABI=0 -static-libstdc++ -std=c++11

.PHONY: clean

clean:
ifeq ($(OS),Windows_NT)
	del -r *.out *.o
else
	rm -r *.out *.o
endif
