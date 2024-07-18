all: attack
clean:
	rm -f attack
attack: attack.c
	g++ -g -pthread -std=c++11 -O0 -o $@ $@.c

