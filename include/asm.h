#ifndef ASM_H
#define ASM_H
#include <stdint.h>
// #include "asm_functions.h"
#include "rtc_emit.h"

enum embed_func{
   embed_func_idiv = 0xFFFF,
   embed_func_udiv = 0xFFFE,
   embed_func_clz32 = 0xFFFD,
   embed_func_i32load = 0xFFFC,
   embed_func_i64load = 0xFFFB,
   embed_func_i32store = 0xFFFA,
   embed_func_i32store16 = 0xFFF9,
   embed_func_i32store8 = 0xFFF8,
   embed_func_i64store = 0xFFF7,
   embed_func_umul = 0xFFF6,
   embed_func_print_stack = 0xFFF5,
   embed_func_count = 0xFFF4,
};
u32 udiv(u32 input1,u32 input2);
i32 idiv(i32 input1,i32 input2);
u32 umul(u32 input1,u32 input2);
u32 LeadingZeros_32(u32 x);
void print_stack();
void count_func();
extern u32 __attribute__((section (".wait"))) global_counter;
// PUSHREF
#define emit_x_PUSHREF8(reg)                     emit_ST_XINC(reg)
#define emit_x_POPREF8(reg)                      emit_LD_DECX(reg)
#define emit_x_CALL_without_saving_RX(target)    emit_2_CALL(target)

void emit_x_CALL(uint16_t target);
void emit_x_POP_32bit(uint8_t base);
void emit_x_POP_16bit(uint8_t base);
void emit_x_POP_REF(uint8_t base);
void emit_x_PUSH_32bit(uint8_t base);
void emit_x_PUSH_16bit(uint8_t base);
void emit_x_PUSH_REF(uint8_t base);

void emit_BRANCH(uint16_t opcode, uint8_t offset);
void emit_MOVW(uint8_t destreg, uint8_t srcreg);

void emit_x_avroraBeep(uint8_t beep);
void emit_x_avroraPrintPC();
void emit_x_avroraPrintRegs();

void emit_x_preinvoke();
void emit_x_postinvoke();

// void emit_save_Y();
// void emit_restore_Y();
// void emit_init_Y();
void emit_local_init(u16 numLocalBytes);
void emit_local_deinit(u16 numLocalBytes);


#define R0        0
#define R1        1
#define ZERO_REG  1
#define R2        2
#define R3        3
#define R4        4
#define R5        5
#define R6        6
#define R7        7
#define R8        8
#define R9        9
#define R10      10
#define R11      11
#define R12      12
#define R13      13
#define R14      14
#define R15      15
#define R16      16
#define R17      17
#define R18      18
#define R19      19
#define R20      20
#define R21      21
#define R22      22
#define R23      23
#define R24      24
#define R25      25
#define R26      26
#define R27      27
#define R28      28
#define R29      29
#define R30      30
#define R31      31

#define RX       26
#define RY       28
#define RZ       30
#define RXL      (RX)
#define RYL      (RY)
#define RZL      (RZ)
#define RXH      (RX+1)
#define RYH      (RY+1)
#define RZH      (RZ+1)

#define Y  1
#define Z  0 

#define SP_L_address 0x5D
#define SP_H_address 0x5E
#define SP_L_port 0x3D
#define SP_H_port 0x3E


// 0000 00kk kkkk k000
#define makeBranchOffset(offset) ( \
                ((offset) & 0x7F) << 3)

// 0000 00r0 0000 rrrr, with d=dest register, r=source register
#define makeSourceRegister(src_register) ( \
               ((src_register) & 0x0F) \
            + (((src_register) & 0x10) << 5))

// 0000 0AA0 0000 AAAA
#define makeIN_OUTport(address) ( \
               ((address) & 0x0F) \
            + (((address) & 0x30) << 5))

// Define of basic Branch
void emit_BRANCH(uint16_t opcode, uint8_t offset);

// ADC                                  0001 11rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_ADC                      0x1C00
#define emit_ADC(destreg, srcreg)       emit_opcodeWithSrcAndDestRegOperand(OPCODE_ADC, destreg, srcreg)

// ADD                                  0000 11rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_ADD                      0x0C00
#define emit_ADD(destreg, srcreg)       emit_opcodeWithSrcAndDestRegOperand(OPCODE_ADD, destreg, srcreg)

// ADIW                                 1001 0110 KKdd KKKK, with d=r24, r26, r28, or r30
#define OPCODE_ADIW                     0x9600
#define emit_ADIW(reg, constant)        emit_ADIW(reg, constant)

// AND                                  0010 00rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_AND                      0x2000
#define emit_AND(destreg, srcreg)       emit_opcodeWithSrcAndDestRegOperand(OPCODE_AND, destreg, srcreg)

// ASR                                  1001 010d dddd 0101
#define OPCODE_ASR                      0x9405
#define emit_ASR(reg)                   emit_opcodeWithSingleRegOperand(OPCODE_ASR, reg)

// BREAK                                1001 0101 1001 1000
#define OPCODE_BREAK                    0x9598
#define emit_BREAK()                    emit(OPCODE_BREAK)

// BREQ                                 1111 00kk kkkk k001, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
#define OPCODE_BREQ                     0xF001
#define emit_BREQ(offset)               emit_BRANCH(OPCODE_BREQ, offset)

// BRGE                                 1111 01kk kkkk k100, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
#define OPCODE_BRGE                     0xF404
#define emit_BRGE(offset)               emit_BRANCH(OPCODE_BRGE, offset)

// BRLO                                 1111 00kk kkkk k000, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
#define OPCODE_BRLO                     0xF000
#define emit_BRLO(offset)               emit_BRANCH(OPCODE_BRLO, offset)

// BRLT                                 1111 00kk kkkk k100, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
#define OPCODE_BRLT                     0xF004
#define emit_BRLT(offset)               emit_BRANCH(OPCODE_BRLT, offset)

// BRNE                                 1111 01kk kkkk k001, with k the signed offset to jump to, in WORDS, not bytes. If taken: PC <- PC + k + 1, if not taken: PC <- PC + 1
#define OPCODE_BRNE                     0xF401
#define emit_BRNE(offset)               emit_BRANCH(OPCODE_BRNE, offset)

// BRPL                                 1111 01kk kkkk k010
#define OPCODE_BRPL                     0xF402
#define emit_BRPL(offset)               emit_BRANCH(OPCODE_BRPL, offset)

// BRSH                                 1111 01kk kkkk k000
#define OPCODE_BRSH                     0xF400
#define emit_BRSH(offset)               emit_BRANCH(OPCODE_BRSH, offset)

//BRCS                                  1111 00kk kkkk k000
#define OPCODE_BRCS                     0xF000
#define emit_BRCS(offset)               emit_BRANCH(OPCODE_BRCS, offset)

//BRCC                                  1111 01kk kkkk k000
#define OPCODE_BRCC                     0xF400
#define emit_BRCC(offset)               emit_BRANCH(OPCODE_BRCC, offset)

//BRMI                                  1111 00kk kkkk k010
#define OPCODE_BRMI                     0xF002
#define emit_BRMI(offset)               emit_BRANCH(OPCODE_BRMI, offset)

// CALL                                 1001 010k kkkk 111k
//                                      kkkk kkkk kkkk kkkk
// TODO: support addresses > 128K
#define OPCODE_CALL                     0x940E
#define emit_2_CALL(address)            emit2(OPCODE_CALL, address)

// CLR
#define emit_CLR(destreg)               emit_EOR(destreg, destreg)

// COM                                  1001 010d dddd 0000, with d=dest register
#define OPCODE_COM                      0x9400
#define emit_COM(reg)                   emit_opcodeWithSingleRegOperand(OPCODE_COM, reg)

// CP                                   0001 01rd dddd rrrr, with r,d=the registers to compare
#define OPCODE_CP                       0x1400
#define emit_CP(destreg, srcreg)        emit_opcodeWithSrcAndDestRegOperand(OPCODE_CP, destreg, srcreg)

// CPC                                  0000 01rd dddd rrrr, with r,d=the registers to compare
#define OPCODE_CPC                      0x0400
#define emit_CPC(destreg, srcreg)       emit_opcodeWithSrcAndDestRegOperand(OPCODE_CPC, destreg, srcreg)

// CPI                                  0011 KKKK dddd KKKK
#define OPCODE_CPI                      0x3000
#define emit_CPI(reg, constant)         emit_LDI_SBCI_SUBI_CPI(OPCODE_CPI, reg, constant)

// DEC                                  1001 010d dddd 1010
#define OPCODE_DEC                      0x940A
#define emit_DEC(reg)                   emit_opcodeWithSingleRegOperand(OPCODE_DEC, reg)

// EOR                                  0010 01rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_EOR                      0x2400
#define emit_EOR(destreg, srcreg)       emit_opcodeWithSrcAndDestRegOperand(OPCODE_EOR, destreg, srcreg)

// IJMP                                 1001 0100 0000 1001
#define OPCODE_IJMP                     0x9409
#define emit_IJMP()                     emit(OPCODE_IJMP)

// IN                                   1011 0AAd dddd AAAA, with d=dest register, A the address of the IO location to read 0<=A<=63 (63==0x3F)
#define OPCODE_IN                       0xB000
#define asm_const_IN(destreg, port)     (OPCODE_IN \
                                         + ((destreg) << 4) \
                                         + makeIN_OUTport(port))
#define emit_IN(destreg, port)          emit(asm_const_IN(destreg, port))

// INC                                  1001 010d dddd 0011, with d=dest register
#define OPCODE_INC                      0x9403
#define emit_INC(reg)                   emit_opcodeWithSingleRegOperand(OPCODE_INC, reg)

// JMP                                  1001 010k kkkk 110k
//                                      kkkk kkkk kkkk kkkk, with k the address in WORDS, not bytes. PC <- k
// TODO: support addresses > 128K
#define SIZEOF_JMP                      4
#define OPCODE_JMP                      0x940C
#define emit_2_JMP(address)             emit2(OPCODE_JMP, address/2)

// LD Rd, -X                            1001 000d dddd 1110, with d=dest register
#define OPCODE_LD_DECX                  0x900E
#define emit_LD_DECX(reg)               emit_opcodeWithSingleRegOperand(OPCODE_LD_DECX, reg)

// LD Rd, -Y                            1001 000d dddd 1010, with d=dest register
#define OPCODE_LD_DECY                  0x900A
#define emit_LD_DECY(reg)               emit_opcodeWithSingleRegOperand(OPCODE_LD_DECY, reg)

// LD Rd, X+                            1001 000d dddd 1101
#define OPCODE_LD_XINC                  0x900D
#define emit_LD_XINC(reg)               emit_opcodeWithSingleRegOperand(OPCODE_LD_XINC, reg)

// LD Rd, Z                             1000 000d dddd 0000
#define OPCODE_LD_Z                     0x8000
#define emit_LD_Z(reg)                  emit_opcodeWithSingleRegOperand(OPCODE_LD_Z, reg)

// LD Rd, Z+                            1001 000d dddd 0001
#define OPCODE_LD_ZINC                  0x9001
#define emit_LD_ZINC(reg)               emit_opcodeWithSingleRegOperand(OPCODE_LD_ZINC, reg)

// LDD                                  10q0 qq0d dddd yqqq, with d=dest register, q=offset from Y or Z, y=1 for Y 0 for Z
#define OPCODE_LDD                      0x8000
#define emit_LDD(reg, yz, offset)       emit_LDD(reg, yz, offset)

// LDI                                  1110 KKKK dddd KKKK, with K=constant to load, d=dest register-16 (can only load to r16-r31)
#define OPCODE_LDI                      0xE000
#define emit_LDI(reg, constant)         emit_LDI_SBCI_SUBI_CPI(OPCODE_LDI, reg, constant)

// LDS                                  1001 000d dddd 0000
//                                      kkkk kkkk kkkk kkkk
#define OPCODE_LDS                      0x9000
#define emit_2_LDS(reg, address)        emit2( asm_opcodeWithSingleRegOperand(OPCODE_LDS, reg), address )

// LPM Rd, Z+                           1001 000d dddd 0101
#define OPCODE_LPM_ZINC                 0x9005
#define emit_LPM_ZINC(reg)              emit_opcodeWithSingleRegOperand(OPCODE_LPM_ZINC, reg)

// LSL
#define emit_LSL(destreg)               emit_ADD(destreg, destreg)


// LSR                                  1001 010d dddd 0110
#define OPCODE_LSR                      0x9406
#define emit_LSR(reg)                   emit_opcodeWithSingleRegOperand(OPCODE_LSR, reg)

// MOV                                  0010 11rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_MOV                      0x2C00
#define asm_MOV(destreg, srcreg)        asm_opcodeWithSrcAndDestRegOperand(OPCODE_MOV, destreg, srcreg)
#define emit_MOV(destreg, srcreg)       emit_opcodeWithSrcAndDestRegOperand(OPCODE_MOV, destreg, srcreg)

// MOVW                                 0000 0001 dddd rrrr, with d=dest register/2, r=source register/2
#define OPCODE_MOVW                     0x0100
#define asm_MOVW(destreg, srcreg)       asm_MOVW(destreg, srcreg)
#define asm_const_MOVW(destreg, srcreg) (((OPCODE_MOVW) + (((destreg)/2) << 4) + makeSourceRegister((srcreg)/2)))
#define emit_MOVW(destreg, srcreg)      emit_MOVW(destreg, srcreg)

// MUL                                  1001 11rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_MUL                      0x9C00
#define emit_MUL(destreg, srcreg)       emit_opcodeWithSrcAndDestRegOperand(OPCODE_MUL, destreg, srcreg)

// NOP                                  0000 0000 0000 0000
#define OPCODE_NOP                      0x0000
#define emit_NOP()                      emit(OPCODE_NOP)

// OR                                   0010 10rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_OR                       0x2800
#define emit_OR(destreg, srcreg)        emit_opcodeWithSrcAndDestRegOperand(OPCODE_OR, destreg, srcreg)

// OUT                                  1011 1AAr rrrr AAAA, with r=src register, A the address of the IO location to read 0<=A<=63 (63==0x3F)
#define OPCODE_OUT                      0xB800
#define asm_const_OUT(port, srcreg)     (OPCODE_OUT \
                                         + ((srcreg) << 4) \
                                         + makeIN_OUTport(port))
#define emit_OUT(port, srcreg)          emit(asm_const_OUT(port, srcreg))


// PUSH                                 1001 001d dddd 1111, with d=source register
#define OPCODE_PUSH                     0x920F
#define emit_PUSH(reg)                  emit_opcodeWithSingleRegOperand(OPCODE_PUSH, reg)
#define asm_const_PUSH(reg)             (((OPCODE_PUSH) + ((reg) << 4)))

// POP                                  1001 000d dddd 1111
#define OPCODE_POP                      0x900F
#define emit_POP(reg)                   emit_opcodeWithSingleRegOperand(OPCODE_POP, reg)
#define asm_const_POP(reg)              (((OPCODE_POP) + ((reg) << 4)))

// RCALL                                1101 kkkk kkkk kkkk, with k relative in words, not bytes. PC <- PC + k + 1
#define OPCODE_RCALL                    0xD000
#define emit_RCALL(offset)              emit(OPCODE_RCALL + offset)
#define asm_const_RCALL(offset)         (OPCODE_RCALL + offset)

// RET                                  1001 0101 0000 1000
#define OPCODE_RET                      0x9508
#define emit_RET()                      emit(OPCODE_RET)

// RJMP                                 1100 kkkk kkkk kkkk, with k the signed offset to jump to, in WORDS, not bytes. PC <- PC + k + 1
#define SIZEOF_RJMP                     2
#define OPCODE_RJMP                     0xC000
#define emit_RJMP(offset)               emit(OPCODE_RJMP + (((offset)/2) & 0xFFF))

// ROL
#define emit_ROL(reg)                   emit_ADC(reg, reg)

// ROR                                  1001 010d dddd 0111
#define OPCODE_ROR                      0x9407
#define emit_ROR(reg)                   emit_opcodeWithSingleRegOperand(OPCODE_ROR, reg)

// SBC                                  0000 10rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_SBC                      0x0800
#define emit_SBC(destreg, srcreg)       emit_opcodeWithSrcAndDestRegOperand(OPCODE_SBC, destreg, srcreg)

// SBCI                                 0100 KKKK dddd KKKK, with K a constant <= 255,d the destination register - 16
#define OPCODE_SBCI                     0x4000
#define emit_SBCI(reg, constant)        emit_LDI_SBCI_SUBI_CPI(OPCODE_SBCI, reg, constant)

// SBIW                                 1001 0111 KKdd KKKK, with d=r24, r26, r28, or r30
#define OPCODE_SBIW                     0x9700
#define emit_SBIW(reg, constant)        emit_SBIW(reg, constant)

// SBRC                                 1111 110r rrrr 0bbb, with r=a register and b=the bit to test
#define OPCODE_SBRC                     0xFC00
#define emit_SBRC(reg, bit)             emit(OPCODE_SBRC + (reg << 4) + bit)

// SBRS                                 1111 111r rrrr 0bbb, with r=a register and b=the bit to test
#define OPCODE_SBRS                     0xFE00
#define emit_SBRS(reg, bit)             emit(OPCODE_SBRS + (reg << 4) + bit)

// ST -X, Rs                            1001 001r rrrr 1110, with r=the register to store
#define OPCODE_ST_DECX                  0x920E
#define emit_ST_DECX(reg)               emit_opcodeWithSingleRegOperand(OPCODE_ST_DECX, reg)

// ST X+, Rs                            1001 001r rrrr 1101, with r=the register to store
#define OPCODE_ST_XINC                  0x920D
#define emit_ST_XINC(reg)               emit_opcodeWithSingleRegOperand(OPCODE_ST_XINC, reg)

// ST Y+, Rs                            1001 001r rrrr 1001, with r=the register to store
#define OPCODE_ST_YINC                  0x9209
#define emit_ST_YINC(reg)               emit_opcodeWithSingleRegOperand(OPCODE_ST_YINC, reg)

// ST Z, Rs                             1000 001r rrrr 0000, with r=the register to store
#define OPCODE_ST_Z                     0x8200
#define emit_ST_Z(reg)                  emit_opcodeWithSingleRegOperand(OPCODE_ST_Z, reg)

// ST Z+, Rs                            1001 001r rrrr 0001, with r=the register to store
#define OPCODE_ST_ZINC                  0x9201
#define emit_ST_ZINC(reg)               emit_opcodeWithSingleRegOperand(OPCODE_ST_ZINC, reg)

// STD                                  10q0 qq1r rrrr yqqq, with r=source register, q=offset from Y or Z, y=1 for Y 0 for Z
#define OPCODE_STD                      0x8200
#define emit_STD(reg, yz, offset)       emit_STD(reg, yz, offset)

// STS                                  1001 001d dddd 0000
//                                      kkkk kkkk kkkk kkkk
#define OPCODE_STS                      0x9200
#define emit_2_STS(address, reg)        emit2( asm_opcodeWithSingleRegOperand(OPCODE_STS, reg), address )

// SUB                                  0001 10rd dddd rrrr, with d=dest register, r=source register
#define OPCODE_SUB                      0x1800
#define emit_SUB(destreg, srcreg)       emit_opcodeWithSrcAndDestRegOperand(OPCODE_SUB, destreg, srcreg)

// SUBI                                 0101 KKKK dddd KKKK, with K a constant <= 255,d the destination register - 16
#define OPCODE_SUBI                     0x5000
#define emit_SUBI(reg, constant)        emit_LDI_SBCI_SUBI_CPI(OPCODE_SUBI, reg, constant)

#define OPCODE_CLI                      0x94f8
#define emit_CLI()                      emit(OPCODE_CLI)

#endif // ASM_H