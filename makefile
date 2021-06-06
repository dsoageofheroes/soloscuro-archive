SRCS    = $(wildcard src/*.c)
DEPS    = $(wildcard src/*.h)
OBJS    = $(SRCS:src/%.c=obj/%.o)
LIB_EXT = so
CFLAGS  = -g -I src/ -I ext/ -L./ -Werror -Wall -O2 -DDEBUG=1 -fPIC
COVFLAGS  = -g ${CFLAGS} -O0 -fprofile-arcs -ftest-coverage

.PHONY: clean test all

include makefile.in
