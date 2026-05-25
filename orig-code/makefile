# makefile for fishnet (MSDOS version), modified from automatic rename
# and used by IBM 19/10/89.

$(FISHNET.obj) : fishNET.c input.h net_type.h error.h learn.h fishNET.h
$(input.obj)   : input.c net_type.h error.h fishNET.h
$(learn.obj)   : learn.c net_type.h error.h learn.h fishNET.h
$(show.obj)    : show.c net_type.h error.h show.h fishNET.h
$(error.obj)   : error.c net_type.h error.h fishNET.h

FishNET.exe : fishNET.obj input.obj learn.obj show.obj error.obj
	del FishNET.lib
	echo FishNET  >> FishNET.obj
	echo input    >> FishNET.obj
	echo learn    >> FishNET.obj
	echo show     >> FishNET.obj
	echo error    >> FishNET.obj
	lib FishNET @FishNET.obj

#
# Unix Makefile for fishnet (BSD/SysV Unix systems)
# and used by IBM 19/10/89.
#
# FISHNET2 = fishNET.c input.c net_type.h error.h learn.h fishNET.h
#
# fishnet2 : fishNET.c input.c learn.c show.c error.c
#	cc -o fishnet2 fishNET.c input.c learn.c show.c error.c -lm
#
# learn.c  : learn.c net_type.h error.h learn.h fishNET.h
# input.c  : input.c net_type.h error.h fishNET.h
# show.c   : show.c  net_type.h error.h show.h fishNET.h
# error.c  : error.c net_type.h error.h fishNET.h

OBJS = fishNET.o input.o learn.o show.o error.o

fishnet : $(OBJS)
	cc -o fishnet $(OBJS) -lm

fishNET.o : fishNET.c input.h net_type.h error.h learn.h fishNET.h
	cc -c fishNET.c

input.o : input.c net_type.h error.h fishNET.h
	cc -c input.c

learn.o : learn.c net_type.h error.h learn.h fishNET.h
	cc -c learn.c

show.o : show.c net_type.h error.h show.h fishNET.h
	cc -c show.c

error.o : error.c net_type.h error.h fishNET.h
	cc -c error.c

clean :
	rm -f $(OBJS) fishnet
