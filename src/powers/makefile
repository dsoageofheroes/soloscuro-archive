SPELL_SRCS = $(wildcard *.c)
SPELL_OBJS = $(SPELL_SRCS:%.c=obj/%.o)

all: obj $(SPELL_OBJS) obj

obj:
	mkdir -p obj

obj/%.o : %.c
	$(CC) -c -g -O2 $(CFLAGS) $< -o $@

clean: 
	rm -rf obj/
