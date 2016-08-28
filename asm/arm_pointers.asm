	AREA   STARTUPCODE, CODE, AT 0x0
code32

arm_tasks:
	b start

Func_Pointers		dd 0
buffer			dd 0
count			dd 0
offsets			db 0, 0, 0, 0, 0, 0, 0, 0

start:
	mov r12, lr

	ldr r0, buffer
	mov r1, #64
	mov lr, pc
	ldr pc, Func_Pointers
	movs r10, r0
	beq at_e

	mov r5, r10
	ldr r3, buffer		; r3 = task pointers
	add r2, r3, #256	; r2 = result buffer
at_l:
	ldr r1, [r3],#4
	adr r6, offsets		; r6 = offsets
	ldr r7, count		; r7 = offset count
at_ll:
	ldrb r4, [r6],#1
	ldr r0, [r1,r4]
	str r0, [r2],#4
	subs r7, r7, #1
	bne at_ll

	subs r5, r5, #1
	bne at_l

at_e:
	mov r0, r10
	bx r12

END
