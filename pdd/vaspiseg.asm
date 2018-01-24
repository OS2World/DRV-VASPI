;;; vaspiseg.asm

;;; virtual ASPI driver for OS/2 2.x
;;; (C) 1994 ARS Computer und Consulting GmbH

;;; Author:  Kai Uwe Rommel <rommel@jonas>
;;; Created: Wed Dec 22 1993

;;; $Id: vaspiseg.asm,v 1.6 1994/02/07 19:28:47 rommel Exp $
;;; $Revision: 1.6 $

;;; $Log: vaspiseg.asm,v $
;;; Revision 1.6  1994/02/07 19:28:47  rommel
;;; bug fix in debugging code
;;;
;;; Revision 1.5  1994/02/06 21:55:15  rommel
;;; first release
;;;
;;; Revision 1.4  1994/02/02 13:20:27  rommel
;;; beta release
;;;
;;; Revision 1.3  1994/01/16 12:00:38  rommel
;;; Cleanup
;;;
;;; Revision 1.2  1994/01/12 15:30:24  rommel
;;; Corrected segment definitions
;;;
;;; Revision 1.1  1994/01/09 20:24:50  rommel
;;; Initial revision

		include devhdr.inc
		.286p

HEADER   	segment dword public 'DATA'
HEADER		ends
_DATA		segment dword public 'DATA'
_DATA		ends
CONST		segment dword public 'CONST'
CONST		ends
_BSS		segment dword public 'BSS'
_BSS		ends
c_common	segment dword public 'BSS'
c_common	ends
EDATA		segment dword public 'BSS'
		public	__edata, __acrtused
__edata		label byte
__acrtused	label byte
EDATA		ends

DGROUP		group HEADER, _DATA, CONST, _BSS, c_common, EDATA

_TEXT		segment dword public 'CODE'
_TEXT		ends
CODE		segment dword public 'CODE'
CODE		ends
ETEXT		segment dword public 'CODE'
		public	__etext
__etext		label byte
ETEXT		ends

CGROUP		group _TEXT, CODE, ETEXT

HEADER   	segment
		dd		-1
		dw		DEV_CHAR_DEV+DEVLEV_1+DEV_GIOCTL+DEV_30
		dw		OFFSET _STRATEGY
		dw		0
		db		"VASPIDD$"
		dw		0
		dw		0
		dw		0
		dw		0
		dd		0
		dw		0
HEADER		ends

_TEXT		segment
	
		extrn	_DriverEntry:NEAR

_STRATEGY	proc    far
		push	es
		push	bx
		call	_DriverEntry
		pop	bx
		pop	es
		mov	word ptr es:[bx+3],ax
		ret
_STRATEGY	endp

_TEXT		ends

		end
