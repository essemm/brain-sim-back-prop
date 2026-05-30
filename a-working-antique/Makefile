# Makefile for fishnet (8087 version), modified from automatic version
# Last mod by SAM 13/10/88.
#

.c.o:
        cc -c -O $*.c

fishNET.o : fishNET.c net_type.h show.h input.h learn.h error.h

input.o : input.c net_type.h error.h learn.h show.h fishNET.h

learn.o : learn.c net_type.h show.h input.h error.h fishNET.h

show.o : show.c net_type.h error.h fishNET.h

error.o : error.c net_type.h fishNET.h show.h learn.h

fishNET : fishNET.o input.o learn.o show.o error.o
        cc -o fishNET fishNET.o input.o learn.o show.o error.o -lm
