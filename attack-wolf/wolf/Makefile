CFLAGS := -g -Wall
LIBS := -lwolfssl
SRCS := $(wildcard *.c)
PROGS := $(patsubst %.c, %, $(SRCS))
LIBDIR := /usr/local/lib
.PHONY: all clean
all: $(PROGS)
%: %.c
	g++ $(CFLAGS) -o $@ $< $(LIBS) -L$(LIBDIR) 
clean: 
	rm $(PROGS)
