AREA   STARTUPCODE, CODE

	MRC     p15, 0, R0,c1,c0
	MRC     p15, 0, R1,c2,c0
	mov		r3, sp
	BX      LR

END
