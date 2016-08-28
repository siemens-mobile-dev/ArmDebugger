AREA   STARTUPCODE, CODE,  at 0x013c8e14
code32

;Copyright 2005 (C) Single, Chaos

; AT+CGSN:A0000000,00000008
; Read data from memory address A0000000 Lenght 8 Bytes
; (If Size is not specified then read 4 bytes)
; Size and address should be multiple of 4
; answer: 0xA1 <n bytes of data>

; AT+CGSN@A08E8DE4,0000001A,00004225
; Call address A08E8DE4, R0 = 0000001A, R1 = 00004225 (R0-R7 Can be given)
; answer: 0xA1 <R0-R12 Registers Dump and CPSR>
 
; AT+CGSN*A80B180011223344....
; Write Data to RAM. address A80B1800 (written as 44332211)
; Address and length should be multiple of 4
; Max Size 128 bytes per command
; answer: 0xA1

; AT+CGSN%A0000000A0000004A0000008....
; Query addresses. Return values (dwords) for each address.
; answer: 0xA1 <data>

AT_CGSN:
		b start

OriginalATHandle 			dd 0xA0ADC20B
GetATCommandLine 			dd 0xA0A9C2D9
ATEmptyBuffer				dd 0xA0A9D5A9
StoreASCIIZR0toSendLater		dd 0xA0A9D393
AfterATCommand				dd 0xA0A9BC75
StoreDataToSendLater			dd 0xA0A9D393

Buffer1				EQU 0x8FE00
Buffer2				EQU 0x8FF10

start:
		stmfd   sp!, {r1-r12,lr}

		mov		lr, pc
		ldr		pc, GetATCommandLine

		stmfd   sp!, {r0}
		mov		lr, pc
		LDR		pc, ATEmptyBuffer
		ldmfd   sp!, {r5}

		ldrb r1, [r5],#1
		cmp r1, #0x20
		bcs IsCmd

		ldmfd sp!, {r1-r12,lr}
		LDR PC, OriginalATHandle

IsCmd:
		mov r0, #0x0d
		strb r0, [r5,#-1]

		swi 0
		; save data_abort, DAC
		mov r2, #0x10	; data abort address
		ldr r2, [r2]
		mrc p15, 0, r3, c3, c0, 0
		stmfd sp!, {r2,r3}

		; set access control register
		mov r0, #0xffffffff
		mcr p15, 0, r0, c3, c0, 0

		cmp r1, #0x40	; "@"
		beq CmdCall

		; set data abort instruction
		mov r2, #0x10
		ldr r0, =0xe25ef004
		str r0, [r2]
		; imb
		mcr p15, 0, r2, c7, c10, 1
		mcr p15, 0, r0, c7, c10, 4
		mcr p15, 0, r2, c7, c5, 1

		cmp r1, #0x25	; "%"
		beq CmdQuery

		bl StrToHex

		cmp r1, #0x3a	; ":"
		beq CmdDump
		cmp r1, #0x2a	; "*"
		beq CmdWrite
		mov r7, #0
		b DumpAndExit

CmdDump:
		mov r6, r0

		ldrb r2, [r5],#1
		cmp r2, #0x2c
		movne r0, #4
		bleq StrToHex
		mov r7, r0

		; r6 = address
		; r7 = size
DumpAndExit:
		swi 0
		; set access control register
		mov r0, #0xffffffff
		mcr p15, 0, r0, c3, c0, 0

		adr r0, s_A1
		mov r1, #1
		mov lr, pc
		ldr pc, StoreDataToSendLater

		movs r4, r7
		beq dne_s
		ldr r3, =Buffer1
dne_l:
		mov r0, #0xffffffff
		ldr r0, [r6]
		add r6, r6, #4
		str r0, [r3],#4
		subs r7, r7, #4
		bne dne_l

		ldr r0, =Buffer1
		mov r1, r4
		mov lr, pc
		ldr pc, StoreDataToSendLater

dne_s:
		mov lr, pc
		ldr pc, AfterATCommand

		swi 0

		ldmfd sp!, {r2,r3}
		mov r1, #0x10
		str r2, [r1]

		; imb
		mcr p15, 0, r1, c7, c10, 1
		mcr p15, 0, r0, c7, c10, 4
		mcr p15, 0, r1, c7, c5, 1

		mcr p15, 0, r3, c3, c0, 0

		ldmfd sp!, {r1-r12,pc}


CmdCall:
		bl StrToHex
		stmfd sp!, {r0}
		ldr r2, =Buffer2
cc_rr:
		ldrb r0, [r5],#1
		cmp r0, #0x2c
		bne cc_rrq
		bl StrToHex
		str r0, [r2],#4
		b cc_rr
cc_rrq:
		mcr p15, 0, r0, c7, c10, 4
		mcr p15, 0, r0, c7, c5, 0

		ldr r0, =Buffer2
		ldmia r0, {r0-r7}
		; ---
		mov lr, pc
		ldmfd sp!, {pc}
		; ---
		ldr lr, =Buffer2
		stmia lr!, {r0-r12}
		mrs r0, CPSR
		stmia lr, {r0}

		ldr r6, =Buffer2
		mov r7, #56
		b DumpAndExit

CmdWrite:
		mov r2, r0
cw_l:
		ldrb r0, [r5]
		cmp r0, #0x30
		bcc cw_x
		bl StrToHex
		ldr r3, =0xff00ff00
		and r1, r3, r0,ROR#8
		and r3, r3, r0,ROR#16
		orr r0, r1, r3,ROR#8
		str r0, [r2]
		add r2, r2, #4
		b cw_l
cw_x:
		mov r7, #0
		b DumpAndExit

CmdQuery:
		ldr r2, =Buffer2
		mov r1, #0
cq_l:
		ldrb r0, [r5]
		cmp r0, #0x30
		bcc cq_q
		bl StrToHex
		ldr r0, [r0]
		str r0, [r2],#4
		add r1, r1, #4
		b cq_l
cq_q:
		mov r7, r1
		ldr r6, =Buffer2
		b DumpAndExit

StrToHex:
		stmfd sp!, {r1-r2,lr}

		mov r0, #0
		mov r2, #8
sth_l:
		ldrb r1, [r5],#1
		cmp r1, #0x60
		subhi r1, r1, #0x20
		sub r1, r1, #0x30
		cmp r1, #9
		subhi r1, r1, #7
		add r0, r1, r0,LSL#4
		subs r2, r2, #1
		bne sth_l

		ldmfd sp!, {r1-r2,pc}


s_A1		db 0xa1

END
