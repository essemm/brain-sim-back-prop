# Makefile for fishnet (8087 version), modified from automatic version
# Last mod by SAM 13/10/88.
#

.c.obj:
 	qcl -c  -W3 -Gs -Za -AC -FPi87 $*.c

fishNET.obj : fishNET.c net_type.h show.h input.h learn.h error.h

input.obj : input.c net_type.h error.h learn.h show.h fishNET.h

learn.obj : learn.c net_type.h show.h input.h error.h fishNET.h

show.obj : show.c net_type.h error.h fishNET.h

error.obj : error.c net_type.h fishNET.h show.h learn.h

Fishnet.exe : fishnet.obj input.obj learn.obj show.obj error.obj 
 	del Fishnet.lnk
 	echo fishnet.obj+ >>Fishnet.lnk
 	echo input.obj+ >>Fishnet.lnk
 	echo learn.obj+ >>Fishnet.lnk
 	echo show.obj+ >>Fishnet.lnk
 	echo error.obj  >>Fishnet.lnk
 	echo Fishnet.exe >>Fishnet.lnk
 	echo Fishnet.map >>Fishnet.lnk
 	link @Fishnet.lnk /NOI $(LDFLAGS);
