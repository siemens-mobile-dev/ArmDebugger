AREA STARTUPCODE, CODE, AT 0x0 
PUBLIC __startup 
EXTERN CODE32 (main) 
__startup PROC CODE32 
b main 
ENDP 
END 
