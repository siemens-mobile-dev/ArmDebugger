AREA   STARTUPCODE, CODE, AT 0x40
code32

fillL2T_start:	// r0=page, r1=addr

	mov r2, #0x100
ac_l:
	str r0, [r1],#4
	add r0, r0, #0x1000
	subs r2, r2, #1
	bne ac_l

	bx lr

END
