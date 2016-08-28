AREA   STARTUPCODE, CODE, AT 0x0
code32

; Search for function signature

nu_search:
	b start

; here will be placed search results

FunctionPointer		dd 0

; bounds of search area

SearchStart	dd 0
SearchEnd	dd 0

; signature

SigLength	dd 0
SigOffset	dd 0

;  (<length> bytes) signature data
;  (<length> bytes) signature mask

FunctionSignature	dd 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
			dd 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0


start:
	adr r12, FunctionPointer

	ldr r10, SigLength	; r10 = signatute length
	adr r8, FunctionSignature
	add r9, r8, r10		; r9 = signature mask
	ldr r5, SearchStart	; r5 = current address
	ldr r7, SearchEnd	; r7 = search end address
	ldr r4, =0x051f0000	; r4 = "ldr rN, =..."
ns_il:
	mov r6, #0		; r6 = signature offset
ns_l:
	cmp r5, r7
	bcs ns_cs
	ldr r0, [r5], #4
	ldr r1, [r9,r6]
	and r1, r0, r1
	ldr r2, [r8,r6]
	cmp r1, r4
	beq ns_ld
	cmp r1, r2
ns_cc:
	subne r5, r5, r6
	bne ns_il
	add r6, r6, #4
	cmp r6, r10
	bne ns_l
ns_ok:
	sub r0, r5, r6
	ldr r1, SigOffset
	cmp r1, #1
	subne r0, r0, r1
	bxne lr

	ldr r1, =0xe92d4000	; stmfd sp!, {...,lr}
	ldr r2, =0xffffc000	; mask
ns_sl:
	ldr r4, [r0,#-4]!
	and r4, r4, r2
	cmp r4, r1
	bne ns_sl
	bx lr
ns_cs:
	mov r0, #0
	bx lr

ns_ld:
	ldr r1, =0xfff
	and r1, r0, r1
	tst r0, #0x00800000
	subeq r0, r5, r1
	addne r0, r5, r1
	tst r0, #0x03
	bne ns_cc
	add r0, r0, #4
	ldr r0, [r0]
	cmp r0, r2
	b ns_cc

END
