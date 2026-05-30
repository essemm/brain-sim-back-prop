# Makefile for fishNET
# Modernised from the original 1988 DOS version.
# Scanned by LLM from the accompanying IMG_7021.png file which is a photo of the original thesis document page

CC     = cc
CFLAGS = -O2 -Wall -Wno-unused-result

OBJS = fishnet.o input.o learn.o show.o error.o

.c.o:
	$(CC) -c $(CFLAGS) $*.c

fishNET : $(OBJS)
	$(CC) -o fishNET $(OBJS) -lm

fishnet.o : fishnet.c net_type.h show.h input.h learn.h error.h
input.o   : input.c net_type.h error.h learn.h show.h fishnet.h
learn.o   : learn.c net_type.h show.h input.h error.h fishnet.h
show.o    : show.c net_type.h error.h fishnet.h
error.o   : error.c net_type.h fishnet.h show.h learn.h

clean:
	rm -f $(OBJS) fishNET

.PHONY: clean
