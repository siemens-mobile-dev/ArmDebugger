AREA   STARTUPCODE, CODE

MAIN proc code32

	; disable interrupts
	mrs r0, CPSR
	orr r0, r0, #0xc0
	msr CPSR_c, r0

	; enable write to einit-protected registers
	mov r0, #0
	bl set_einit

	; disable first watchdog
	ldr r1, =0xf4400000
	mov r0, #0x08
	str r0, [r1, #0x28]

	; disable write to einit-protected registers
	mov r0, #1
	bl set_einit

	ldr r1, =0xf4400000
	ldr r0, [r1, #0x7c]
	bic r0, r0, #1
	str r0, [r1, #0x7c]

	bl init_watchdog

	bl init_F0

	bl read_info

	bl set_115200

	; answer: OK
	mov r0, #0xa5
	bl tx_byte

main_loop:
	bl rx_byte
	cmp r0, #0x41	; 'A'
	beq cmd_ack
	cmp r0, #0x54	; 'T' test speed
	beq cmd_test
	cmp r0, #0x51	; 'Q'
	beq cmd_quit
	b main_loop
	; ----------

cmd_ack:
	mov r0, #'R'
	bl tx_byte
	b main_loop
	; ----------

cmd_quit:
	; fast power-off
	bl switch_watchdog
	bl switch_watchdog
q_l:	b q_l
	; ----------


cmd_test:
	; address
	bl rx_byte
	mov r6, r0
	bl rx_word
	mov r7, r0

	mov r0, #'A'
	bl tx_byte

	mov r0, #0x800
cc_l0:
	subs r0, r0, #1
	bne cc_l0

	mov r2, #0xf1000000
	mov r0, r7, LSR#16
	str r0, [r2, #0x14]
	mov r0, r7, LSL#16
	mov r0, r0, LSR#16
	str r0, [r2, #0x18]

cc_w:
	bl rx_byte_w
	cmp r0, #'t'
	beq cc_c
	cmp r0, #0xffffffff
	bne cc_w

	bl set_115200
	b main_loop

cc_c:
	mov r0, #'a'
	bl tx_byte

	mov r7, #4096
cc_l:
	mov r0, r6
	bl tx_byte
	subs r7, r7, #1
	bne cc_l

	mov r0, #0x800
ct_l2:
	subs r0, r0, #1
	bne ct_l2

	bl set_115200

	b main_loop
	; ----------

endp


set_115200 proc code32

; uses r0, r1, r2

	ldr r1, =0x000c01d8
	mov r2, #0xf1000000
	mov r0, r1, LSR#16
	str r0, [r2, #0x14]
	mov r0, r1, LSL#16
	mov r0, r0, LSR#16
	str r0, [r2, #0x18]

	bx lr

endp

; --------------------------

tx_byte proc code32

; void tx_byte(r0=byte)
; uses r0, r1, r2

	mov r2, #0xf1000000
	ldr r1, [r2, #0x20]
	bic r1, r1, #0xff
	orr r1, r0, r1
	str r1, [r2, #0x20]
tx_w:
	ldr r1, [r2, #0x68]
	ands r1, r1, #0x02
	beq tx_w

	ldr r1, [r2, #0x70]
	orr r1, r1, #2
	str r1, [r2, #0x70]

	b serve_watchdog

endp

; --------------------------

rx_byte_w proc code32

; byte rx_byte_w()
; uses r0, r1, r2, r3, r4

	mov r4, #0x20000
rxw_loop:
	mov r3, lr
	bl serve_watchdog
	mov lr, r3
	mov r1, #0xf1000000
	ldr r0, [r1, #0x68]
	ands r0, r0, #0x04
	bne rx_c
	subs r4, r4, #1
	bne rxw_loop

	mov r0, #0xffffffff
	bx lr

endp

; --------------------------

rx_byte proc code32

; byte rx_byte()
; uses r0, r1, r2, r3, r4

rx_loop:
	mov r3, lr
	bl serve_watchdog
	mov lr, r3
	mov r1, #0xf1000000
	ldr r0, [r1, #0x68]
	ands r0, r0, #0x04
	beq rx_loop

rx_c:
	ldr r0, [r1, #0x70]
	orr r0, r0, #4
	str r0, [r1, #0x70]

	ldr r0, [r1, #0x24]
	and r0, r0, #0xff
	bx lr

endp

; ------------------------

rx_word proc code32

; word rx_word()
; uses r0, r1, r2, r3, r4, r5

	mov r5, lr

	bl rx_byte
	mov r4, r0,LSL#24
	bl rx_byte
	orr r4, r4, r0,LSL#16
	bl rx_byte
	orr r4, r4, r0,LSL#8
	bl rx_byte
	orr r0, r4, r0

	bx r5

endp

; ----------------------------

set_einit proc code32

; void set_einit(r0=on/off)
; uses r0, r1, r2, r3

	ldr r3, =0xf4400000
	ldr r1, [r3, #0x24]
	bic r1, r1, #0x0e
	orr r1, r1, #0xf0
	ldr r2, [r3, #0x28]
	and r2, r2, #0x0c
	orr r1, r1, r2
	str r1, [r3, #0x24]

	bic r1, r1, #0x0d
	orr r1, r1, #2
	orr r0, r0, r1
	str r0, [r3, #0x24]

	bx lr

endp

; -------------------------

init_F0 proc code32

; void init_F0()
; uses r0, r1

	MOV     R1, #0xF0000000
	LDR     R0, =0xA8000041
	STR     R0, [R1,#0x88]
	LDR     R0, =0x30720200
	STR     R0, [R1,#0xC8]
	MOV     R0, #6
	STR     R0, [R1,#0x40]
	LDR     R0, =0x891C70
	STR     R0, [R1,#0x50]
	MOV     R0, #0x23
	STR     R0, [R1,#0x60]
	LDR	R0, =0xA0000021
	STR	R0, [R1,#0xA0]
	LDR	R0, =0x00520600
	STR	R0, [R1,#0xE0]

	BX LR

endp

; -------------------------

read_info proc code32

; void read_info()
;

	mov r5, lr

	ldr r1, =0xa8100000
	mov r2, #128
	mov r0, #0
ri_clr:
	str r0, [r1], #4
	subs r2, r2, #4
	bne ri_clr

	; phone info
	ldr r1, =0xa8100000
	ldr r0, =0xa0000210
	mov r2, #32
	bl memcpy
	ldr r0, =0xa000065c
	mov r2, #16
	bl memcpy

	; flash base
	ldr r4, =0xa8100000
	mov r0, #0xa0000000
	str r0, [r4, #0x40]

	; flash id
	mov r1, #0xa0000000

	mov r0, #0x90
	bl send_flash_cmd
	ldrh r0, [r1, #0x0]
	strh r0, [r4, #0x50]
	bl send_flash_reset

	mov r0, #0x90
	bl send_flash_cmd
	ldrh r0, [r1, #0x2]
	strh r0, [r4, #0x52]
	bl send_flash_reset


	; cfi information
	mov r0, #0x98
	strh r0, [r1, #0xaa]
	nop

	add r2, r4, #0x54
	ldrh r0, [r1, #0x4e]
	strb r0, [r2],#1
	add r1, r1, #0x54
cfi_l:
	ldrh r0, [r1],#2
	strb r0, [r2],#1
	and r0, r1, #0xff
	cmp r0, #0x6a
	bne cfi_l

	mov r0, #0xff
	strh r0, [r1]

	bx r5

endp

; -------------------------

memcpy proc code32

; uses r0, r1, r2, r3
; r0=src
; r1=dst
; r2=len

sc_loop:
	ldrb r3, [r0], #1
	strb r3, [r1], #1
	subs r2, r2, #1
	bne sc_loop

	bx lr

endp

; --------------------------

send_flash_cmd proc code32

; uses r0, r1, r2, r3
; r0 = command
; r1 = flash base

	mov r3, #0xaa
	add r2, r1, #0xa00
	strh r3, [r2, #0xaa]

	mov r3, #0x55
	add r2, r1, #0x500
	strh r3, [r2, #0x54]

	add r2, r1, #0xa00
	strh r0, [r1, #0xaa]

	bx lr

endp

; -------------------------

send_flash_reset proc code32

; uses r0, r1
; r1 = flash base

	mov r0, #0xf0
	strh r0, [r1]
	mov r0, #0xff
	strh r0, [r1]

	bx lr

endp

; -------------------------

init_watchdog proc code32

; void init_watchdog()
; uses r0, r1, r2, r3, r4

                LDR     R0, =0xF4300000 
                MOV     R2, #0x100      
                STR     R2, [R1]        

		LDR     R0, =0xF4400000
		LDR     R0, [R0,#0x60]
                MOV     R0, R0,LSR#8
                AND     R0, R0, #0xff

		ldr	r1, =0xf4300118
		sub	r2, r1, #0xcc
		
		cmp	r0, #0x1a
		beq	iwd_1a

		add	r1, r1, #0x60
		add	r2, r2, #0x04

iwd_1a:
		add	r3, r2, #0x4
		sub	r4, r3, #0x0c
		mov	r12, r1

                MOV     R0, #1          
                STR     R0, [R2]
                MOV     R0, #0x10
                STR     R0, [R3]
                MOV	R0, #0x500
                STR     R0, [R1]
                MOV	R0, #0x4000
		ORR	R0, R0, #0x510
                STR     R0, [R4]

		LDR     R1, 0xF4B00020
		LDR     R11, [R1]

		b switch_watchdog

endp

; -------------------------

serve_watchdog proc code32

; void serve_watchdog()
; uses r0, r1, r2

	ldr r2, =0xf4b00020
	ldr r0, [r2]
	sub r1, r0, r11
	cmp r1, #0x200
	bcc swd_exit
	mov r11, r0
	b switch_watchdog

endp

; ------------------------

switch_watchdog proc code32

; void switch_watchdog()
; uses r0, r1, r2

	LDR     R0, [R12]        
	MOV     R0, R0,LSL#22   
	LDR     R2, [R12]        
	MVN     R0, R0,LSR#31   
	BIC     R2, R2, #0x200
	AND     R0, R0, #1      
	ORR     R0, R2, R0,LSL#9
	STR     R0, [R12]        

swd_exit:

	BX LR

endp

END