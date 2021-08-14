CPP=/usr/bin/g++
C = /usr/bin/gcc

all: clean bison flex nutshell

nutshell:
	$(CPP) -std=c++11 -o nutshell nutshell.cpp lex.yy.c nutshell.tab.c command_handler.cpp

bison:
	bison -d nutshell.y

flex:
	flex nutshell.l

clean:
	rm -f nutshell lex.yy.c nutshell.tab.c nutshell.tab.h
