/**
 * @file
 * @brief Forth VM Opcodes (for Bytecode Assembler)
 */
#ifndef __EFORTH_OPCODE_H
#define __EFORTH_OPCODE_H

#define OPCODES \
	OP(EXIT),   \
	OP(ENTER),  \
    OP(BYE),    \
    OP(QRX),    \
    OP(TXSTO),  \
    OP(DOLIT),  \
    OP(DOVAR),  \
    OP(EXECU),  \
    OP(DOES),   \
    OP(DONEXT), \
    OP(QBRAN),  \
    OP(BRAN),   \
    OP(STORE),  \
    OP(PSTOR),  \
    OP(AT),     \
    OP(CSTOR),  \
    OP(CAT),    \
    OP(RFROM),  \
    OP(RAT),    \
    OP(TOR),    \
    OP(DROP),   \
    OP(DUP),    \
    OP(SWAP),   \
    OP(OVER),   \
    OP(ROT),    \
    OP(PICK),   \
    OP(AND),    \
    OP(OR),     \
    OP(XOR),    \
    OP(INV),    \
    OP(LSH),    \
    OP(RSH),    \
    OP(ADD),    \
    OP(SUB),    \
    OP(MUL),    \
    OP(DIV),    \
    OP(MOD),    \
    OP(NEG),    \
    OP(GT),     \
    OP(EQ),     \
    OP(LT),     \
    OP(ZGT),    \
    OP(ZEQ),    \
    OP(ZLT),    \
    OP(ONEP),   \
    OP(ONEM),   \
    OP(QDUP),   \
    OP(DEPTH),  \
    OP(RP),     \
        OP(BL),    \
        OP(CELL),  \
        OP(ABS),   \
        OP(MAX),   \
        OP(MIN),   \
        OP(WITHIN),\
        OP(TOUPP), \
        OP(COUNT), \
    OP(ULESS),  \
    OP(UMMOD),  \
    OP(UMSTAR), \
    OP(MSTAR),  \
        OP(UMPLUS),\
        OP(SSMOD), \
        OP(SMOD),  \
        OP(MSLAS), \
        OP(S2D),   \
        OP(D2S),   \
    OP(DABS),   \
    OP(DNEG),   \
    OP(DADD),   \
    OP(DSUB),   \
    OP(UDSMOD), \
        OP(SPAT),  \
        OP(TRC),   \
        OP(SAVE),  \
        OP(LOAD),  \
        OP(CALL),  \
        OP(CLK),   \
        OP(PIN),   \
        OP(MAP),   \
        OP(IN),    \
        OP(OUT),   \
        OP(AIN),   \
        OP(PWM),   \
        OP(TMISR), \
        OP(PCISR), \
        OP(TMRE),  \
        OP(PCIE)
#endif // __EFORTH_OPCODE_H
