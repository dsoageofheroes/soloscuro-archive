MAIN    = src/main.c
SRCS    = $(filter-out $(MAIN), $(wildcard src/*.c))
DEPS    = $(wildcard src/*.h)
OBJS    = $(SRCS:src/%.c=obj/%.o)
LIB_EXT = so
CFLAGS  = -g -I src/ -I ext/ -L./ -Werror -Wall -llua5.1 -lxmimidi -O2 -DDEBUG=1 -fPIC
EXEC    = mdark

.PHONY: clean test all

include makefile.in
