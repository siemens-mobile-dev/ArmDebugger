AREA   STARTUPCODE, CODE
code32

                MOV     R2, #0x1000

l1:
                LDR     R0, [R1],#4
                CMP     R0, #0
                BNE     xit
                SUBS    R2, R2, #4
                BNE     l1

xit:
                BX      LR

END

