AREA STARTUPCODE,CODE, at 0x8e324

pf_vector:
	b pf_start

da_vector:
	b da_start

ui_vector:
	b ui_start

arm_clearTLB:
	b clearTLB_start

arm_IMB:
	b IMB_start

arm_copy:
	b copy_start

arm_fillL2T:
	b fillL2T_start

arm_fastoff:
	b fastoff_start

arm_read:
	b read_start

arm_search:
	b search_start

arm_exec:
	b exec_start

arm_reserved1:
	nop

FlashBase	dd 0
QRamBase	dd 0
XRamBase	dd 0

FlashShift	db 0
QRamShift	db 0
SnapNo		db 0
XRamShift	db 0

LogBuffer	dd 0

LogCurrent	db 0
InBreak		db 0
		db 0
		db 0

LogLastRead	dd 0
LostCount	dd 0

SnapData	dd 0
SnapMask	dd 0

WatchPage	dd 0
WatchStart	dd 0
WatchEnd	dd 0

ServiceSP	dd 0
ServiceLR	dd 0

TempExecPage	dd 0
HolderPos	dd 0
JumpPos		dd 0
TimerPort	dd 0xf4b00010

chk_cond:
		nop
		nop
		cmp r0, r1
		bx lr
		b pf_exit1
		dd 0

pf_start:

	stmfd sp!, {r0-r3}

	;check, is this a real abort or bkpt instruction

	ldr r1, FlashBase	; flash area
	ldrb r2, FlashShift
	cmp r1, lr,LSR r2
	ldrne r1, QRamBase	; current RAM area
	ldrneb r2, QRamShift
	cmpne r1, lr,LSR r2
	ldrne r1, XRamBase	; external RAM area
	ldrneb r2, XRamShift
	cmpne r1, lr,LSR r2
	ldrne r1, TempExecPage	; page(s) with temp.code
	eorne r1, r1, lr
	movnes r1, r1,LSR#13
	bne real_pfa

breakpt:

	; get the number of snappoint
	; (common 4 bits for ARM and thumb
	; bkpt instruction)

	ldrb r2, [lr,#-4]
	ands r2, r2, #0x0f
	strb r2, SnapNo

	; position in the snapinfo array

	ldr r3, SnapData
	add r3, r3, r2,LSL#5
	add r3, r3, r2,LSL#4
	str r3, JumpPos

	; if we're already in breakpoint
	; and snapNo!=0, skip

	ldrb r0, InBreak
	cmp r0, #0
	cmpne r2, #0
	bne pf_exit1

	; if inBreak=3 (snapNo=0),
	; clear bkpt state and return

	cmp r0, #3
	streqb r2, InBreak	;  r2=0
	ldmeqfd sp!, {r0-r3}
	moveqs pc, lr

	; check condition

	cmp r2, #0
	beq b_nc
	add lr, r3, #24
	ldmfd sp, {r0-r3}
	dd 0xe12fff3e		; blx lr
	b pf_exit1 		; if condition not matches
b_nc:

	; if no space to record, skip

	ldrb r2, LogCurrent
	ldrb r0, LogLastRead
	cmp r0, r2
	beq pf_lostevt

	; set lr to current log block,
	; update LogCurrent

	ldr lr, LogBuffer
	add lr, lr, r2,LSL#7
	add r2, r2, #1
	and r2, r2, #0x1f
	strb r2, LogCurrent

	; store registers to log

	ldmfd sp!, {r0-r3}
	stmia lr, {r0-r14}^
	nop
	stmfd sp!, {r0-r3}

	; store snapNo,SPSR,timer

	ldr r1, TimerPort
	ldr r0, [r1]
	bic r0, r0, #0xff000000
	ldrb r1, SnapNo
	orr r0, r0, r1,LSL#24
	mrs r1, SPSR
	and r1, r1, #0xf0000000
	orr r0, r0, r1
	str r0, [lr,#60]
	
	; if needed, record 64 bytes of memory

	ldr r1, JumpPos
	ldr r1, [r1,#16]
	cmp r1, #0xffffffff
	beq simple

	; if request<0x1000, it's [rN+offset],
	; otherwise it's an address

	cmp r1, #0x2000
	bcs smem_address
smem_register:
	mov r2, r1,LSL#24	; offset<<22
	mov r1, r1,LSR#8
	tst r1, #0x10
	bic r1, r1, #0x10
	ldr r1, [lr,r1,LSL#2]	; register value
	beq smem_sgptr			; single pointer
	cmp r1, #0
	beq simple
	ldr r1, [r1,r2,ASR#22]	; address
	mov r2, #0
smem_sgptr:
	cmp r1, #0		; test for NULL ptr
	beq simple
	add r1, r1, r2,ASR#22	; address



smem_address:
	add lr, lr, #64
	mov r2, #64
d_l:
	ldrb r0, [r1],#1	; copying 64 bytes
	strb r0, [lr],#1
	subs r2, r2, #1
	bne d_l
	sub lr, lr, #128
simple:

	; is this a breakpoint?

	ldrb r1, SnapNo
	ldr r0, SnapMask
	orr r0, r0, #1
	add r1, r1, #1
	movs r0, r0,LSR r1
	bcc pf_exit1

	; yes, breakpoint handler

	; clear thumb bit

	mrs r1, SPSR
	bic r1, r1, #0x20
	msr SPSR_cxsf, r1

	; internal or external?

	ldr r0, [lr,#52]	; saved sp
	ldr r1, TempExecPage
	eor r0, r0, r1		; stack ptr is on
	movs r0, r0,LSR#24	;   Shadow RAM?
	movne r0, #1		; no, internal
	moveq r0, #2		; yes, external
	strb r0, InBreak
	bne int_brk

ext_brk:
	ldmfd sp!, {r0-r3}
arec:				 ; discard stack
	adr lr, ServiceSP	; contents and
	ldmia lr!, {sp}^	; return directly
	ldmia lr, {pc}^		; to CGSN routine

int_brk:
	ldr r1, HolderPos	; infinite loop
	ldr r0, =0xeafffffe	; at HolderPos
	str r0, [r1]
	mcr p15, 0, r0, c7, c10, 4	; IMB
	mcr p15, 0, r0, c7, c5, 0
	ldmfd sp!, {r0-r3}
	adr lr, HolderPos	; now go to Holder
	ldmia lr, {pc}^		; and wait for bkpt 0

pf_lostevt:
	ldr r0, LostCount
	add r0, r0, #1
	str r0, LostCount

pf_exit1:
	ldmfd sp!, {r0-r3}
	adr lr, JumpPos		; jumping to saved
	ldmia lr, {pc}^		; instruction

real_pfa:
	ldr r0, =0x007504ab

real_abt:

	; r0 = type

	ldr r1, TempExecPage	; using as temp RAM
	str lr, [r1,#0x4c]
	mov lr, r1
	mrs r1, SPSR
	MRC p15, 0, r2, c5, c0, 0 ; FSR
	MRC p15, 0, r3, c6, c0, 0 ; FAR
	stmia lr!, {r0-r3}
	ldmfd sp!, {r0-r3}
	stmia lr, {r0-r14}^
	nop

	mrs r1, CPSR
	bic r1, r1, #0xc0
	msr CPSR_c, r1

	ldr r1, TempExecPage	; write to com0
	mov r2, #0x50
	mov r3, #0xF1000000
ra_l:
	ldr r0, [r3, #0x48]
	movs r0, r0, LSR#8
	bne ra_l
	ldrb r0, [r1],#1
	str r0, [r3, #0x20]
	subs r2, r2, #1
	bne ra_l

	ldr r0, [lr,#52]	; saved sp
	ldr r1, TempExecPage
	eor r0, r0, r1		; stack ptr is on
	movs r0, r0,LSR#24	;   Shadow RAM?
ra_zz:
	bne ra_zz			; no - harakiri
	b arec			; yes - try to recover

da_start:

	stmfd sp!, {r0-r3,lr}
	sub lr, lr, #4

	mrc p15, 0, r3, c6, c0, 0 ; FAR
	ldr r0, WatchPage
	cmp r0, r3,LSR#12
	ldrne r0, =0x017504ab
	bne real_abt

	ldr r0, WatchStart
	cmp r3, r0
	ldrcs r0, WatchEnd
	cmpcs r0, r3
	bcc w_skip

	ldrb r2, LogCurrent
	ldrb r0, LogLastRead
	cmp r0, r2
	beq w_skip
	ldr r1, LogBuffer
	add r1, r1, r2,LSL#7
	add r2, r2, #1
	and r2, r2, #0x1f
	strb r2, LogCurrent
	mov r0, #0
	str r0, [r1,#60]
	str r3, [r1]
	str lr, [r1,#56]

w_skip:
	MRC p15, 0, r1, c3, c0, 0 ; saving DAC
	str r1, SaveDAC
	mov r1, #0xffffffff
	MCR p15, 0, r1, c3, c0, 0 ; enable all access
	mrs r1, SPSR
	str r1, SaveSPSR2
	orr r1, r1, #0x1f
	msr SPSR_c, r1
	ands r1, r1, #0x20
	subne lr, lr, #1
	str lr, RetAddr
	subne lr, lr, #1
	adreq r1, iArm
	adrne r1, iThumb	; +1
	str r1, [sp,#0x10]
	ldreq r0, [lr,#-4]
	streq r0, iArm
	ldrneh r0, [lr,#-2]
	strneh r0, (iThumb & ~1)
	MCR p15, 0, r0, c7, c10, 4
	MCR p15, 0, r1, c7, c5, 1
	ldmfd sp!, {r0-r3, pc}^

iArm:
	nop
iExit:
	str r1, SaveR1
	ldr r1, SaveDAC
	MCR p15, 0, r1, c3, c0, 0
	ldr r1, SaveSPSR2
	bic r1, r1, #0x20
	msr CPSR_c, r1
	ldr r1, SaveR1
	ldr PC, RetAddr

	CODE16

iThumb:
	nop
	bx r15

	CODE32

	b iExit

RetAddr	  dd 0
SaveDAC	  dd 0
SaveSPSR2 dd 0
SaveR1	  dd 0

ui_start:

	stmfd sp!, {r0-r3}
	ldr r0, =0x027504ab
	b real_abt

exec_start:
	str sp, ServiceSP
	str lr, ServiceLR
	ldr sp, TempExecPage
	add sp, sp, #4064
	mov lr, pc
	ldr pc, TempExecPage
	adr lr, ServiceSP
	ldmia lr, {sp,pc}

clearTLB_start:

	mrc p15, 0, r15, c7, c14, 3
	bne clearTLB_start

	mcr p15, 0, r0, c7, c10, 4
	mcr p15, 0, r0, c7, c5, 0
	mcr p15, 0, r0, c8, c7, 0

	mrc p15, 0, r1, c2, c0, 0
	bx lr

IMB_start:	// r0=data, r1=address

	STR r0, [r1]
	MCR p15, 0, r1, c7, c10, 1
	MCR p15, 0, r0, c7, c10, 4 
	MCR p15, 0, r1, c7, c5, 1
	bx lr

copy_start:	// r1=src, r2=dest, r3=size

cp_l:
	ldr r0, [r1],#4
	str r0, [r2],#4
	subs r3, r3, #4
	bne cp_l

	bx lr

fillL2T_start:	// r0=page, r1=addr

	mov r2, #0x100
ac_l:
	str r0, [r1],#4
	add r0, r0, #0x1000
	subs r2, r2, #1
	bne ac_l

	bx lr

fastoff_start:
	mov r0, #0
	bx r0

read_start:

	ldr r0, [r1]
	bx lr

search_start:	// r0=what, r1=addr_s, r2=addr_e, r3=length, r4=mask

	mov r9, r0
	mov r10, r4

sr_l0:
	mov r4, r9
	mov r5, r10
	ldrb r7, [r4]
	ldrb r8, [r5]
	mov r6, #0
sr_l1:
	cmp r1, r2
	bcs not_found
	ldrb r0, [r1],#1
	and r0, r0, r8
	cmp r0, r7
	bne sr_l1

	; first matches

sr_l2:
	add r4, r4, #1
	add r5, r5, #1
	add r6, r6, #1
	cmp r6, r3
	beq sr_tak

	ldrb r0, [r1],#1
	ldrb r7, [r4]
	ldrb r8, [r5]
	and r0, r0, r8
	cmp r0, r7
	beq sr_l2

	sub r1, r1, r6
	b sr_l0

sr_tak:
	sub r0, r1, r6
	bx lr

not_found:
	mov r0, #0xffffffff
	bx lr	


END

