;;; code.asm

;;; virtual ASPI driver for OS/2 2.x
;;; (C) 1994 ARS Computer und Consulting GmbH

;;; Author:  Kai Uwe Rommel <rommel@jonas>
;;; Created: Wed Dec 22 1993

;;; $Id: code.asm,v 1.2 1994/03/01 09:09:20 rommel Exp $
;;; $Revision: 1.2 $

;;; $Log: code.asm,v $
;;; Revision 1.2  1994/03/01 09:09:20  rommel
;;; bug fixes for second release
;;;
;;; Revision 1.1  1994/02/06 21:56:47  rommel
;;; Initial revision
;;;

	.model	small,c
	.286p

	.data
	
pwlen   DW      ?
pwbuf   DB      (32) DUP (?)

	.code

	assume	cs:_TEXT
	assume	ds:_DATA

Encode	proc	buffer:ptr byte, bytes:word, password:ptr byte

	pusha

	mov	si,password
	mov	di,offset pwbuf
	
copy:	mov	al,[si]
	mov	[di],al
	inc	si
	inc	di
	or	al,al
	jnz	copy

	dec	di	
	sub	di,offset pwbuf
	mov	pwlen,di

	mov	ax,bytes
	mov	di,buffer
	call	docode

	popa
	ret
	
Encode	endp
	
docode 	proc
 
        mov     dx,ax
	add	dx,di
 
nextw:  push    di
        push    dx
	call    makepw
        pop     dx
        pop     di
	
        mov     si,offset pwbuf
	mov	cx,dx
	sub	cx,di
	cmp	cx,pwlen
	jbe	nextc
        mov     cx,pwlen
 
nextc:  mov     al,[si]
        xor     [di],al
        inc     di
        inc     si
        loop    nextc
 
        cmp     di,dx
        jb      nextw
 
        ret
 
docode	endp

makepw  proc
 
        mov     bx,pwlen
        mov     cx,bx
        mov     di,offset pwbuf
        mov     si,di
        add     si,cx
        dec     si
 
loop1:  rol     byte ptr [si],3
        mov     al,[di]
        xor     al,[si]
        xor     al,01010101b
        mov     [di],al
        inc     di
        dec     si
        loop    loop1
 
        mov     cx,bx
        mov     si,offset pwbuf
 
loop2:  rcl     byte ptr [si],1
        inc     si
        loop    loop2
 
        mov     si,offset pwbuf
        pushf
        shr     byte ptr [si],1
        popf
        rcl     byte ptr [si],1
 
        ret
 
makepw  endp
 
	end
