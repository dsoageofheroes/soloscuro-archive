SRCS    = $(wildcard src/*.c)
DEPS    = $(wildcard src/*.h)
OBJS    = $(SRCS:src/%.c=obj/%.o)
LIB_EXT = so
CFLAGS  = -g -I include/ src/ -I ext/ -L./ -Werror -Wall -O2 -DDEBUG=1 -fPIC
COVFLAGS  = -g ${CFLAGS} -O0 -fprofile-arcs -ftest-coverage

.PHONY: clean test all builds test generate install

all: builds

test: builds
	#cd build ; ctest --verbose
	cd build ; ctest

builds: generate build/Makefile
	cd build ; make -j4

generate: src/powers/wizard

src/powers/wizard:
	cd code-generation/powers; gcc -o extract extract.c
	mkdir -p src/powers/wizard
	mkdir -p src/powers/priest
	mkdir -p src/powers/psionic
	mkdir -p src/powers/innate
	cd code-generation/powers; ./extract ../../src/powers/wizard/ ../../src/powers/priest/ ../../src/powers/psionic/ ../../src/powers/innate/

clean:
	rm -rf build

build/Makefile:
	mkdir -p build
	cd build/ ; cmake -DCMAKE_BUILD_TYPE=Debug ..

install:
	cd build ; cmake -DCMAKE_INSTALL_PREFIX=`pwd`/../release/linux .
	cd build ; make install
