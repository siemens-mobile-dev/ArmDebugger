AREA   STARTUPCODE, CODE,  at 0xa0d705b0
code32

;Copyright 2005 (C) Single, Chaos

;AT+CGSN:A0000000,00000008
;Read data from memory address A0000000 Lenght 8 Bytes (If Size is not specified then 
;read 256 bytes)

;AT+CGSN@A08E8DE4,0000001A,00004225
;Call address A08E8DE4, R0 = 0000001A, R1 = 00004225 (R0-R7 Can be given)
;Out is the R0-R12 Registers Dump and CPSR
 
;AT+CGSN*A80B180011223344....
; Write Data to RAM. address A80B1800 
;Max Size 128 bytes per command 

;AT+CGSN%A0000000A0000004A0000008....
; Query addresses. Return values for each address in one line.

AT_CGSN:
		b start

OriginalATHandle 			dd 0xA0ADC20B
GetATCommandLine 			dd 0xA0A9C2D9
ATEmptyBuffer				dd 0xA0A9D5A9
StoreASCIIZR0toSendLater	dd 0xA0A9D393
AfterATCommand				dd 0xA0A9BC75

Buffer				EQU 0x8FD00
Buffer2				EQU 0X8FF00

start:
		stmfd   sp!, {r0-r12,lr}

		mov		lr, pc
		ldr		pc, =GetATCommandLine

		stmfd   sp!, {r0}
		mov		lr, pc
		LDR		pc, =ATEmptyBuffer
		ldmfd   sp!, {r5}

		ldrb r1, [r5],#1

		cmp r1, #0x5e	; '^'
		bne At115

		ldrb r0, [r5],#1
		sub r0, r0, #0x30
		adr r1, iSpeeds
		ldr r1, [r1, r0,LSL#2]

		mov r2, #0xf1000000
		mov r0, r1, LSR#16
		str r0, [r2, #0x14]
		mov r0, r1, LSL#16
		mov r0, r0, LSR#16
		str r0, [r2, #0x18]

		ldrb r1, [r5],#1

At115:
		cmp r1, #0x20
		bcs IsCmd

		ldmfd sp!, {r0-r12,lr}
		LDR PC, OriginalATHandle

IsCmd:
		; save CPSR, data_abort, DAC
		mrs r0, CPSR
		swi 0
		mov r2, #0x10	; data abort address
		ldr r2, [r2]
		mrc p15, 0, r3, c3, c0, 0
		stmfd sp!, {r0,r2,r3}

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
		b Exit


;	115200, 230400, 460800, 921600, 1228800
iSpeeds	dd 0x000c01d8, 0x000501b4, 0x00000092, 0x00000127, 0x0000018a

Digits	db "0123456789ABCDEF"

CmdDump:
		mov r1, r0

		ldrb r0, [r5],#1
		cmp r0, #0x2c
		movne r0, #256
		bleq StrToHex

		; r0 = size
		; r1 = address
DumpAndExit:
		ldr r2, =Buffer
		adr r5, Digits

dne_m:
		mov r3, #0xffffffff
		ldr r3, [r1]
		add r1, r1, #4
		mov r6, #4
dne_l:
		mov r4, r3,LSR#4
		and r4, r4, #0x0f
		ldrb r4, [r5,r4]
		strb r4, [r2],#1
		and r4, r3, #0x0f
		ldrb r4, [r5,r4]
		strb r4, [r2],#1
		mov r3, r3,ROR#8
		subs r6, r6, #1
		bne dne_l

		subs r0, r0, #4
		bne dne_m

		strb r0, [r2]

		ldr r0, =Buffer
		mov lr, pc
		ldr pc, =StoreASCIIZR0toSendLater

Exit:		
		swi 0
		; set access control register
		mov r0, #0xffffffff
		mcr p15, 0, r0, c3, c0, 0

		ldmfd sp!, {r0,r2,r3}
		mov r1, #0x10
		str r2, [r1]

		; imb
		mcr p15, 0, r1, c7, c10, 1
		mcr p15, 0, r0, c7, c10, 4
		mcr p15, 0, r1, c7, c5, 1

		mcr p15, 0, r3, c3, c0, 0
		msr CPSR_c, r0

		mov lr, pc
		ldr pc, =AfterATCommand

		ldmfd sp!, {r0-r12,pc}


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

		mov r0, #56
		ldr r1, =Buffer2
		b DumpAndExit

CmdWrite:
		mov r2, r0
cw_l:
		ldrb r0, [r5]
		cmp r0, #0x30
		bcc Exit
		bl StrToHex
		ldr r3, =0xff00ff00
		and r1, r3, r0,ROR#8
		and r3, r3, r0,ROR#16
		orr r0, r1, r3,ROR#8
		str r0, [r2]
		add r2, r2, #4
		b cw_l

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
		mov r0, r1
		ldr r1, =Buffer2
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


END
