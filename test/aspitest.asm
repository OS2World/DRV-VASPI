	.model 	small
	.stack 	00100H

	.data
	
devname	db	"SCSIMGR$",0
handle	dw	0
entry	dd	0
srb	db	58 dup (?)

msg1	db	"SCSI manager: '"
msg1len equ	$-msg1
msg2	db	"Host adapter: '"
msg2len equ	$-msg2
nl	db	"'",13,10

	.code

msg	macro	text,length
	mov	ax,04000H
	mov	bx,1
	mov	cx,length
	mov	dx,offset text
	int	021H
	endm

start:	mov	ax,@data
	mov	ds,ax

	mov	ax,03D00H
	lea	dx,devname
	int	021H
	jc	exit
	mov	handle,ax

	mov	bx,ax
	mov	ax,04402H
	lea	dx,entry
	mov	cx,4
	int	021H

	mov	ax,03E00H
	mov	bx,handle
	int	021H

	push	ds
	lea	bx,srb
	push	bx
	lea	bx,entry
	call	dword ptr [bx]
	add	sp,4

	msg	msg1,msg1len
	msg	srb+10,16
	msg	nl,3
	msg	msg2,msg2len
	msg	srb+26,16
	msg	nl,3
	
exit:	mov	ax,04C00H
	int	021H

	end	start
