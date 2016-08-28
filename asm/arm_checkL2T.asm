AREA   STARTUPCODE, CODE, AT 0x40
code32

fillL2T_start:	// r0=page, r1=addr

	mov r2, #0x100
ac_l:
	ldr r3, [r1],#4
	cmp r3, #0
	streq r0, [r1,#-4]
	subs r2, r2, #1
	bne ac_l

	bx lr

END
