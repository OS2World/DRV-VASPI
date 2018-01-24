# Makefile

# virtual ASPI driver for OS/2 2.x
# (C) 1994 ARS Computer und Consulting GmbH

# Author:  Kai Uwe Rommel <rommel@jonas>
# Created: Mon Dec 20 1993

# $Id: makefile,v 1.6 1994/09/14 12:15:47 rommel Exp rommel $
# $Revision: 1.6 $

# $Log: makefile,v $
# Revision 1.6  1994/09/14 12:15:47  rommel
# added real stub for specific DOS sessions
#
# Revision 1.5  1994/03/09 17:35:31  rommel
# simplified rules
#
# Revision 1.4  1994/02/02 13:22:39  rommel
# beta release
#
# Revision 1.3  1994/01/09 20:19:53  rommel
# first somewhat working version
#
# Revision 1.2  1993/12/22 13:05:42  rommel
# Restructured, stub DD added.
#
# Revision 1.1  1993/12/20  16:16:41  rommel
# Initial revision

OBJ = main.obj aspi.obj stubdata.obj
LIBS = vdh.lib

CC32 = icc -Q -Sm -Ss -Sp1 -W2 -Rn -Gr -Id:\dev\include\c\vdd
LD32 = link386 /nologo /align:16 /map:full
LD16 = link /nologo
AS = masm -Ml -p -t -Z -Id:\dev\include\asm

.SUFFIXES: .c .obj

.c.obj:
	$(CC32) -c $<

all: vaspi.sys vaspistb.sys

vaspi.sys: $(OBJ) vaspi.def
	$(LD32) $(OBJ),$@,$*.map,$(LIBS),vaspi.def;
	mapsym $*.map

vaspistb.sys: stub.obj
	$(LD16) stub.obj,stub.drv;
	exe2bin stub.drv $@ /q
	del stub.drv

aspi.obj: aspi.c vaspi.h
main.obj: main.c vaspi.h stub.h

stub.obj: stub.asm
	$(AS) stub.asm,stub.obj,stub.lst;

stub.h: stubdata.asm

stubdata.asm: stub.obj code2dat.cmd
	code2dat stub.lst $@

stubdata.obj: stubdata.asm
	$(AS) stubdata.asm,stubdata.obj;

clean:
	rm -f $(OBJ) stub.obj stub.lst stub.h stubdata.asm \
		vaspi.sys vaspi.map vaspi.sym vaspistb.sys

# end of Makefile
