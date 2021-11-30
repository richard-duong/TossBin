CC=g++
FLAGS=-std=c++17 -Wall -g
INCLUDES=-I lib

test: all
	./bin/toss --list

all: toss

toss:
	$(CC) $(FLAGS) $(INCLUDES) src/main.cpp -o bin/toss

clean:
	rm bin/*
