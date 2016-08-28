AREA   STARTUPCODE, CODE, AT 0x40
code32

;	mov r1, #1
;	mcr p15, 0, r0, c3, c0, 0

;	ldr r1, =0xa80b0010
;	mov r0, #0x5a
;	str r0, [r1]
;	bx lr

Data_x_handler:

	sub lr, lr, #8
	stmfd sp!, {r0-r4,lr}

	mrc p15, 0, r0, c6, c0, 0

	cmp r0, #0xa8000000
	bcc dx_abt
	cmp r0, #0xa9000000
	bcs dx_abt

	mrc p15, 0, r1, c2, c0, 0
	mov r2, r0,LSR#20
	add r1, r1, r2,LSL#2	; r1 = L1TT address

	ldr r1, [r1]		; r1 = L1TT entry
	bic r1, r1, #0x0ff
	bic r1, r1, #0x300
	mov r2, r0,LSR#12
	mov r2, r2,LSL#2
	and r2, r2, #0x3fc
	add r1, r1, r2		; r1 = L2TT address

	orr r0, r0, #0xf00
	orr r0, r0, #0x0fe
	bic r0, r0, #1		; r0 = new L2TT entry

	str r0, [r1]

	mcr p15, 0, r0, c8, c7, 0

	ldmfd sp!, {r0-r4,pc}^

dx_abt:
	b dx_abt

END
