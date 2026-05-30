# Makefile for fishNET
# Modernised from the original 1988 DOS version.

CC     = cc
CFLAGS = -O2 -Wall -Wno-unused-result -Isrc

OBJS = obj/fishnet.o obj/input.o obj/learn.o obj/show.o obj/error.o

obj/%.o : src/%.c | obj
	$(CC) -c $(CFLAGS) $< -o $@

fishNET : $(OBJS)
	$(CC) -o fishNET $(OBJS) -lm

obj :
	mkdir obj

obj/fishnet.o : src/fishnet.c src/net_type.h src/show.h src/input.h src/learn.h src/error.h
obj/input.o   : src/input.c   src/net_type.h src/error.h src/learn.h src/show.h src/fishnet.h
obj/learn.o   : src/learn.c   src/net_type.h src/show.h  src/input.h src/error.h src/fishnet.h
obj/show.o    : src/show.c    src/net_type.h src/error.h src/fishnet.h
obj/error.o   : src/error.c   src/net_type.h src/fishnet.h src/show.h src/learn.h

clean :
	rm -rf obj fishNET

.PHONY : clean
