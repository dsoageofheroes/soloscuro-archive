SDL_SRCS  = $(wildcard *.c)
SDL_SRCS  += $(wildcard windows/*.c)
SDL_DEPS  = $(wildcard *.h)
SDL_OBJS  = $(SDL_SRCS:%.c=obj/%.o)
POWER_LIB = src/powers/obj/*
LIB_EXT   = so
CFLAGS    = -g -O2 -I src/ -I ext/ -L./ -Werror -Wall -llua5.3 -lxmimidi -lm -O2 -DDEBUG=1 -fPIC
SDL_FLAGS = -g -O2 -I ../ -I ext/ -L./ -Llibs/linux/ -Werror -Wall -llua5.3 -lxmimidi -lm -O2 -DDEBUG=1  -lSDL2_ttf -lADLMIDI -lsndfile
LUA_LIB   = -llua5.3

.PHONY: clean test all

sdl: obj/windows $(SDL_OBJS)

obj/%.o : %.c $(DEPS) $(SDL_DEPS)
	$(CC) -c $(SDL_FLAGS) $< -o $@

mdark : obj/windows $(SDL_OBJS) libds.so libxmimidi.so
	$(CC) $(SDL_FLAGS) $(SDL_OBJS) -o $@ -lds -lSDL2

obj/windows:
	mkdir -p obj/windows

valgrind: mdark
	LD_LIBRARY_PATH=./ valgrind --gen-suppressions=all --suppressions=./sup/linux_sdl_gl.sup --leak-check=full --show-leak-kinds=all ./mdark --lua lua/test.lua

clean:
	rm -rf obj
