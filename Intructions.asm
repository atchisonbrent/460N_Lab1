.ORIG x4000
LOL    ADD r1, r2, r3
ABD    ADD r6, r7, #-4
ABC    AND r1, r2, r6
ASM    AND r2, r5, x-F
    BR ASM
    BRnzp LOL
    BRnz ABD
    BRnp ABC
    BRn LOL
    BRzp LOL
    BRz LOL
    BRp LOL
    JMP r2
    JSR ABD
    JSRR r5
    LDB r7, r6, #-10
    LDW r7, r6, #10
    LEA r3, LOL
    NOT r1, r2
    RET
    RTI
    LSHF r1, r2, #3
    RSHFL r3, r4, #7
    RSHFA r4, r5, #1
    STB r3, r4, #-10
    STW r6, r7, #5
    TRAP x25
    XOR r2, r3, r4
    XOR r4, r5, r6
.END
    
   