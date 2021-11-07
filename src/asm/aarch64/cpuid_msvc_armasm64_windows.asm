AREA READ_MRS, CODE, READONLY
EXPORT READ_MRS_S

count SETA 0
WHILE count < 31
    reg_num_x$count SETA count
    count SETA count + 1
WEND
reg_num_xzr SETA 31

READ_MRS_S PROC
    r SETA X0
    sysreg SETA X1
    cpuid SETA DCI 0xD5200000 :OR: sysreg :OR: reg_num_$r
    MOV X0, cpuid
ENDP

END