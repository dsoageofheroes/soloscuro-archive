MAIN   = src/main.c
SRCS   = $(filter-out $(MAIN), $(wildcard src/*.c))
OBJS   = $(SRCS:src/%.c=obj/%.o)
CFLAGS = -g -I src/ -I ext/ -L./ -Werror -Wall -llua5.1 -lxmimidi -O2 -DDEBUG=1
EXEC   = mdark

ifdef OS
	LIB_EXT = dll
else
	LIB_EXT = so
    CFLAGS += -fPIC
endif

.PHONY: clean test all

all: $(EXEC) libds.so

$(EXEC): $(MAIN) libds.so
	gcc $(CFLAGS) $(OBJS) $(MAIN) -o $@

libds.so: $(OBJS) libxmimidi.so
	gcc -L./ $(OBJS) -shared -o libds.$(LIB_EXT) -llua5.1 -lxmimidi

libxmimidi.so:  
	gcc ext/xmimidi.c -shared -o libxmimidi.$(LIB_EXT)

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
