all: fectortest.cpp ../fector.hpp
	g++ -Wall --std=c++11 fectortest.cpp -o fectortest -pedantic \
	-lpthread -march=native -I.. -Wextra -ggdb
	./fectortest
