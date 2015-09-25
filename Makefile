MKDIR_P = mkdir -p
BIN_DIR := bin
LIB_DIR := lib

all:output_dirs \
	shmt.o \
	test_apps

output_dirs: ${BIN_DIR} \
	${LIB_DIR}

${BIN_DIR}:
	${MKDIR_P} ${BIN_DIR}

${LIB_DIR}:
	${MKDIR_P} ${LIB_DIR}

shmt.o:src/shmt.c
	gcc -Iinclude/ -Wall -c -O2 -std=c99 src/shmt.c -o lib/shmt.o

test_apps:create

create:test/create.cc
	g++ -Iinclude/ -Wall test/create.cc -g -o bin/create lib/shmt.o

clean:
	rm -f lib/shmt.o
	rm -f bin/*
