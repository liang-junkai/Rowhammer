all: main offline
.PHONY: all
main: main.c
	g++ -g -o main main.c -l relic
offline: offline.c
	g++ -g -o offline offline.c -l relic
.PHONY: clean
clean:
	rm main offline
