SRCS    = $(wildcard src/*.c)
DEPS    = $(wildcard src/*.h)
OBJS    = $(SRCS:src/%.c=obj/%.o)
LIB_EXT = so
CFLAGS  = -g -I include/ src/ -I ext/ -L./ -Werror -Wall -O2 -DDEBUG=1 -fPIC
COVFLAGS  = -g ${CFLAGS} -O0 -fprofile-arcs -ftest-coverage

.PHONY: clean test all builds test

all: builds

test: builds
	#cd build ; ctest --verbose
	cd build ; ctest

builds: build/libadlmidi/libADLMIDI.so build/Makefile
	cd build ; make -j4

clean:
	rm -rf build

build/Makefile:
	cd build/ ; cmake -DCMAKE_BUILD_TYPE=Debug ..

build/libadlmidi/libADLMIDI.so: ext/libadlmidi/AUTHORS
	mkdir -p build/libadlmidi
	mkdir -p lib
	cd build/libadlmidi ; pwd
	cd build/libadlmidi ; cmake -DCMAKE_BUILD_TYPE=Release -DlibADLMIDI_SHARED=ON ../../ext/libadlmidi
	cd build/libadlmidi ; make -j 8

ext/libadlmidi/AUTHORS:
	git submodule update --init --recursive
