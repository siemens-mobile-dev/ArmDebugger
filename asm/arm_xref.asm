; search xrefs
; input:
;  r0 = type (0:whole)
;  r1 = start address
;  r2 = end address
;  r3 = target
; return:
;  r0 = address (+1 if thumb)
;  r0 = 0xffffffff - not found

AREA   STARTUPCODE, CODE

MAIN proc code32

start:
	cmp r0, #0
	beq x_whole
ex_ff:
	mov r0, #0xffffffff
	bx lr
ex_r1:
	sub r0, r1, #4
	bx lr

x_whole:
	bic r3, r3, #3
	sub r5, r3, r0
	sub r5, r5, #8
	mov r5, r5, LSR#2 ; current b/bl value
xw_l:
	ldr r0, [r1], #4
	; -- check direct value
	bic r4, r0, #3
	cmp r4, r3
	beq ex_r1
	; -- check b/bl/blx
	bic r5, r5, #0xff000000
	bic r4, r0, #0xff000000
	cmp r4, r5
	bne xw_s1
	and r4, r0, #0x0e000000
	cmp r4, #0x0a000000
	beq ex_r1
xw_s1:
	sub r5, r5, #1
	cmp r1, r2
	bcc xw_l
	b ex_ff

endp

END
