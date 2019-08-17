MAIN   = src/main.c
SRCS   = $(filter-out $(MAIN), $(wildcard src/*.c))
OBJS   = $(SRCS:src/%.c=obj/%.o)
MUSIC_OBJS = $(SRCS:src/%music.c=obj/%music.o)
CFLAGS = -g -I . -I src/ -I xmimidi/ -L./ -Lfluidsynth -Werror -Wall -llua5.1 -lxmimidi -lfluidsynth -O2
EXEC   = mdark

ifdef OS
	LIB_EXT = dll
else
	LIB_EXT = so
    CFLAGS += -fPIC
	LINUX_CFLAGS = -fPIC
endif


.PHONY: clean test all

all: $(SRCS) $(EXEC) libds.so music.so

$(EXEC): $(OBJS) $(MAIN) libds.so music.so
	gcc $(CFLAGS) $(OBJS) $(MAIN) -o $@

libds.so: $(OBJS) libxmimidi.so music.so
	gcc -L./ -Lfluidsynth $(OBJS) -shared -o libds.$(LIB_EXT) -llua5.1 -lxmimidi -lfluidsynth

music.so: $(MUSIC_OBJS) 
	gcc -I xmimidi/ $(LINUX_CFLAGS) -I. -L. -Lfluidsynth $(MUSIC_OBJS) -shared -o dsmusic.$(LIB_EXT) -llua5.1 -lxmimidi -lfluidsynth

libxmimidi.so: $(OBJS) 
	gcc xmimidi/*.c $(LINUX_CFLAGS) -shared -o libxmimidi.$(LIB_EXT)

obj/%.o: src/%.c src/%.h obj
	gcc -c $(CFLAGS) $< -o $@

obj/%.o: src/%.c obj
	gcc -c $(CFLAGS) $< -o $@

obj:
	mkdir -p obj

tools: gff-inspector

gff-inspector: $(OBJS) tools/gff-inspector.c
	gcc $(CFLAGS) $(OBJS) tools/gff-inspector.c -o gff-inspector

bin:
	mkdir -p bin

%test: bin
	gcc $(CFLAGS) test/$@.c -o bin/$@ -lcunit -lds -llua5.1
	LD_LIBRARY_PATH=./ bin/$@

%test-lua: bin
	LD_LIBRARY_PATH=./ lua5.1 test/$@.lua -v

clean:
	rm -rf $(EXEC) obj gff-inspector libds.$(LIB_EXT) libxmimidi.$(LIB_EXT) bin/
