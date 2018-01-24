# Makefile

# virtual ASPI driver for OS/2 2.x
# (C) 1994 ARS Computer und Consulting GmbH

# Author:  Kai Uwe Rommel <rommel@jonas>
# Created: Wed Dec 22 1993

# $Id: makefile,v 1.6 1994/03/09 16:53:57 rommel Exp rommel $
# $Revision: 1.6 $

# $Log: makefile,v $
# Revision 1.6  1994/03/09 16:53:57  rommel
# added debug utility
#
# Revision 1.5  1994/03/04 14:40:58  rommel
# changed DDK locations
#
# Revision 1.4  1994/02/07 19:28:47  rommel
# bug fix in debugging code
#
# Revision 1.3  1994/02/06 21:55:15  rommel
# first release
#
# Revision 1.2  1994/02/02 13:20:27  rommel
# beta release
#
# Revision 1.1  1994/01/09 20:24:50  rommel
# Initial revision

INC = ..\..\ddk\inc
H = ..\..\ddk\h
LIB = ..\..\ddk\lib;d:\msc\lib

#AS = masm -MX -T -Z -I. -I$(INC)
AS = ml -nologo -Cp -Zm -I. -I$(INC)
CC = cl -nologo -Zp -Oegcltaz -G2s -Answ -W3 -I. -I$(H)
LINK = link /nologo /map /nod /exepack /packd /a:16 /far
MAPSYM = mapsym

OBJS = vaspiseg.obj vaspidd.obj code.obj
LIBS = dhcalls.lib os2286.lib slibcep.lib
DEF  = vaspidd.def

.asm.obj:
	$(AS) -c $*.asm
.c.obj:
	$(CC) $(CFLAGS) $(LOCAL) -c $*.c

all: vaspidd.sys stamp.exe debug.exe

vaspidd.sys: $(OBJS) $(DEF) makefile
	$(LINK) $(LFLAGS) $(OBJS),$*.sys,$*.map,$(LIBS),$(DEF);
	$(MAPSYM) $*.map

stamp.exe: stamp.c code.obj
	cl -O -o $@ stamp.c code.obj -Lp -link /pm:vio

debug.exe: debug.c
	cl -O -o $@ debug.c -Lp -link /pm:vio

# end of Makefile
