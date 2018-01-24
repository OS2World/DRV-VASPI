;;; stub.asm

;;; virtual ASPI driver for OS/2 2.x
;;; (C) 1994 ARS Computer und Consulting GmbH

;;; Author:  Kai Uwe Rommel <rommel@jonas>
;;; Created: Wed Dec 22 1993

;;; $Id: stub.asm,v 1.12 1994/09/14 12:15:00 rommel Exp rommel $
;;; $Revision: 1.12 $

;;; $Log: stub.asm,v $
;;; Revision 1.12  1994/09/14 12:15:00  rommel
;;; new init method, allows real stub for specific DOS sessions
;;;
;;; Revision 1.11  1994/05/26 12:48:20  rommel
;;; fix for NetWare requester
;;;
;;; Revision 1.10  1994/04/20 11:54:37  rommel
;;; fourth release
;;;
;;; Revision 1.9  1994/03/29 16:09:50  rommel
;;; now using VDHPushInt, simplified
;;;
;;; Revision 1.8  1994/03/09 17:29:00  rommel
;;; new version using VDHPushFarCall
;;;
;;; Revision 1.7  1994/03/03 16:52:08  rommel
;;; changes for WINASPI.DLL
;;;
;;; Revision 1.6  1994/03/01 16:18:53  rommel
;;; second release
;;;
;;; Revision 1.5  1994/03/01 15:30:22  rommel
;;; bug fix for post mode
;;;
;;; Revision 1.4  1994/02/04 17:43:43  rommel
;;; first release
;;;
;;; Revision 1.3  1994/02/02 13:22:39  rommel
;;; beta release
;;;
;;; Revision 1.2  1994/01/27 13:00:02  rommel
;;; first test release
;;;
;;; Revision 1.1  1993/12/22 13:07:23  rommel
;;; Initial revision

	.186
	PAGE	255,132

REQHDR	STRUC
	rh_reqlen   	DB      ?
	rh_unit     	DB      ?
	rh_command  	DB      ?
	rh_status   	DW      ?
	rh_rs2      	DB      8 DUP (?)
	rh_units    	DB      ?
	rh_addr     	DD      ?
	rh_count    	DW      ?
	rh_rs3      	DW	?
REQHDR ENDS

tick	EQU	01CH
int2f	EQU	02FH
ini_int	EQU	05AH

mp_post	EQU	0C701H
mp_aspi	EQU	0C702H
	
CODE	SEGMENT BYTE PUBLIC 'CODE'

        ASSUME CS:CODE,DS:CODE,ES:CODE
        ORG 00000H

;;; The next few lines are used by an REXX script
;;; to convert the listing file into an .ASM file.
;;;
;;; #### HEADERFILE=stub.h
;;; #### ENTRYPOINT=StubStart
;;; #### LENGTH=StubLength
;;;
;;; #### BEGIN EXTRACTION

start:	

header	DD	-1		; link to next driver; -1 = end of list
attrib	DW  	0C800h		; character, ioctl, ibm, removable
	DW  	strat
	DW  	inter
	DB  	'SCSIMGR$'      ; Device name
	DW  	0               ; reserved (must be zero)
	DB  	0               ; drive letter (must be zero)
	DB  	0               ; number of units supported

reqhdr	DD 	?               ; address of Request Header
	
old2f	DD	?		; old multiplex (2F) interrupt vector

vector	DD	?		; address of VASPI hook routine
avail	DW	0		; is ASPI available to this session?

aspi	PROC FAR

	jmp	SHORT entry
	nop
	nop
	nop
	
entry:	push	bp
	mov	bp,sp

	push	bx
	push	es
	
	mov	bx,[bp+6]
	mov	es,[bp+8]

	pushf
	call	cs:vector

	pop	es
	pop	bx
		
	pop	bp
        ret

aspi	ENDP

mplex	PROC FAR

	cmp	ax,mp_aspi
	jne	post

	push	es
	push	bx
	
	mov	es,bx
	mov	bx,0
	pushf
	call	cs:vector
	
	pop	bx
	pop	es
	
	iret

post:	cmp	ax,mp_post
	jne	old

	pusha
	push	ds
	push	es
	
	mov	es,cx
	push	es
	push	bx
	call	DWORD PTR es:[bx+26] 	; post routine offset in SRB
	add	sp,4
	
	pop	es
	pop	ds
	popa
	
	iret
	
old:	jmp	cs:old2f

mplex	ENDP

strat	PROC FAR

        mov	WORD PTR cs:[reqhdr],bx
        mov	WORD PTR cs:[reqhdr+2],es
        ret

strat	ENDP

inter	PROC FAR

        push    ax
        push    bx
        push    es

        les     bx,cs:[reqhdr]
        mov     WORD PTR es:[bx.rh_status],00100H	;  OK
        mov     al,es:[bx.rh_command]

        cmp     al,0
        jz      init
	cmp	al,3
	jz	ioctl
;	cmp	al,13
;	jz	open

exit:	pop     es
        pop     bx
        pop     ax
        ret

init:	mov     WORD PTR es:[bx.rh_addr],OFFSET drvend
        mov     WORD PTR es:[bx.rh_addr+2],cs

	push	es
	push	bx
	mov	ax,mp_aspi
	int	ini_int			; ask VDD for entry point
	cmp	ax,mp_aspi
	jz	novdd
	mov	avail,ax
	mov	WORD PTR cs:vector,bx
	mov	WORD PTR cs:vector+2,es
novdd:	pop	bx
	pop	es

	cli	
	push	0
	pop	es
	
        mov     bx,int2f * 4		; set multiplex (2F) interrupt vector
        push    es:[bx]
        push    es:[bx+2]
	pop	WORD PTR cs:old2f+2
	pop	WORD PTR cs:old2f
	push	OFFSET mplex
	push	cs
	pop     es:[bx+2]
	pop     es:[bx]
	
	sti

        jmp     exit

ioctl:	cmp	avail,1
	jz	ioctl2
        mov     WORD PTR es:[bx.rh_status],08102H	;  ERROR
ioctl2:	les	bx,es:[bx.rh_addr]
	mov	WORD PTR es:[bx],OFFSET aspi
	mov	WORD PTR es:[bx+2],cs
	jmp	exit
	
open:	cmp	avail,1
	jz	open2
        mov     WORD PTR es:[bx.rh_status],08102H	;  ERROR
open2:	jmp	exit

inter	ENDP

drvend	LABEL BYTE

;;;
;;; #### END EXTRACTION
;;;

CODE    ENDS

        END	start

;;; end of stub.asm
