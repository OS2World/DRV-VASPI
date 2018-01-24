# Makefile

# virtual ASPI driver for OS/2 2.x
# (C) 1994 ARS Computer und Consulting GmbH

# Author:  Kai Uwe Rommel <rommel@jonas>
# Created: Wed Mar 02 1994

# $Id: makefile,v 1.4 1994/03/09 15:52:32 rommel Exp $
# $Revision: 1.4 $

# $Log: makefile,v $
# Revision 1.4  1994/03/09 15:52:32  rommel
# added test program
#
# Revision 1.3  1994/03/03 16:51:48  rommel
# first working version
#
# Revision 1.2  1994/03/02 15:43:17  rommel
# first (empty) prototype
#
# Revision 1.1  1994/03/02 14:54:16  rommel
# Initial revision
#

CC = cl -nologo -W3 -O
CFLAGS = -Zep -J -G2sw
LFLAGS = -link /nod /al:16
RC = rcwin

OBJ = winaspi.obj
DEF = winaspi.def
RES = winaspi.res
LLIBS = libentry.obj sdllcew.lib libw.lib
MLIBS = slibcew.lib libw.lib

.SUFFIXES: .c .obj .rc .res

.c.obj:
	$(CC) $(CFLAGS) -c $<

.rc.res:
	$(RC) -r $<

all: winaspi.dll testaspi.exe

winaspi.dll: $(OBJ) $(DEF) $(RES)
	$(CC) $(OBJ) $(DEF) -o $@ $(LLIBS) $(LFLAGS)
	$(RC) $(RES) $@

winaspi.rc:
	echo /* */ >winaspi.rc

testaspi.exe: testaspi.obj testaspi.def winaspi.res
	$(CC) testaspi.obj testaspi.def -o $@ $(MLIBS) $(LFLAGS)
	$(RC) winaspi.res $@

clean:
	rm -f $(OBJ)

# end of Makefile
