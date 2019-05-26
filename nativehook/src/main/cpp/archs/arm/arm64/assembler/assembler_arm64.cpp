//
// Created by swift on 2019/5/11.
//

#include "assembler_arm64.h"

using namespace SandHook::Assembler;
using namespace SandHook::RegistersA64;
using namespace SandHook::AsmA64;

AssemblerA64::AssemblerA64(CodeBuffer* codeBuffer) {
    codeContainer.setCodeBuffer(codeBuffer);
}

void *AssemblerA64::getPC() {
    return reinterpret_cast<void *>(codeContainer.curPc);
}

void *AssemblerA64::getStartPC() {
    return reinterpret_cast<void *>(codeContainer.startPc);
}

void AssemblerA64::allocBufferFirst(U32 size) {
    codeContainer.allocBufferFirst(size);
}

void *AssemblerA64::finish() {
    codeContainer.commit();
    return reinterpret_cast<void *>(codeContainer.startPc);
}


void AssemblerA64::Emit(U64 data64) {
    Emit(reinterpret_cast<Unit<Base>*>(new Data64(data64)));
}

void AssemblerA64::Emit(U32 data32) {
    Emit(reinterpret_cast<Unit<Base>*>(new Data32(data32)));
}

void AssemblerA64::Emit(Unit<Base> *unit) {
    codeContainer.append(unit);
}

void AssemblerA64::MoveWide(RegisterA64 &rd, INST_A64(MOV_WIDE)::OP op, U64 imme,
                                            INST_A64(MOV_WIDE)::Shift shift) {
    Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(MOV_WIDE)(op, &rd, imme, shift)));
}

void AssemblerA64::Mov(WRegister &rd, U32 imme) {
    const U16 h0 = BITS16L(imme);
    const U16 h1 = BITS16H(imme);
    Movz(rd, h0, INST_A64(MOV_WIDE)::Shift0);
    Movk(rd, h1, INST_A64(MOV_WIDE)::Shift1);
}

void AssemblerA64::Mov(XRegister &rd, U64 imme) {
    const U32 wl = BITS32L(imme);
    const U32 wh = BITS32H(imme);
    const U16 h0 = BITS16L(wl);
    const U16 h1 = BITS16H(wl);
    const U16 h2 = BITS16L(wh);
    const U16 h3 = BITS16H(wh);
    Movz(rd, h0, INST_A64(MOV_WIDE)::Shift0);
    Movk(rd, h1, INST_A64(MOV_WIDE)::Shift1);
    Movk(rd, h2, INST_A64(MOV_WIDE)::Shift2);
    Movk(rd, h3, INST_A64(MOV_WIDE)::Shift3);
}

void AssemblerA64::Movz(RegisterA64 &rd, U64 imme,
                                             INST_A64(MOV_WIDE)::Shift shift) {
    MoveWide(rd, INST_A64(MOV_WIDE)::MOV_WideOp_Z, imme, shift);
}

void AssemblerA64::Movk(RegisterA64 &rd, U64 imme,
                                             INST_A64(MOV_WIDE)::Shift shift) {
    MoveWide(rd, INST_A64(MOV_WIDE)::MOV_WideOp_K, imme, shift);
}

void AssemblerA64::Movn(RegisterA64 &rd, U64 imme,
                                             INST_A64(MOV_WIDE)::Shift shift) {
    MoveWide(rd, INST_A64(MOV_WIDE)::MOV_WideOp_N, imme, shift);
}

void AssemblerA64::Br(XRegister &rn) {
    Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(BR_BLR_RET)(INST_A64(BR_BLR_RET)::BR, rn)));
}

void AssemblerA64::B(Off offset) {
    Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(B_BL)(INST_A64(B_BL)::B, offset)));
}

void AssemblerA64::B(Label *label) {
    Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(B_BL)(INST_A64(B_BL)::B, *label)));
}

void AssemblerA64::Bl(Off offset) {
    Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(B_BL)(INST_A64(B_BL)::BL, offset)));
}

void AssemblerA64::Bl(Label *label) {
    Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(B_BL)(INST_A64(B_BL)::BL, *label)));
}

void AssemblerA64::B(Condition condition, Off offset) {
    Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(B_COND)(condition, offset)));
}

void AssemblerA64::B(Condition condition, Label *label) {
    Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(B_COND)(condition, *label)));
}

void AssemblerA64::Tbz(RegisterA64 &rt, U32 bit, Label *label) {
    Emit((reinterpret_cast<Unit<Base> *>(new INST_A64(TBZ_TBNZ)(INST_A64(TBZ_TBNZ)::TBZ, rt, bit, *label))));
}

void AssemblerA64::Tbz(RegisterA64 &rt, U32 bit, Off offset) {
    Emit((reinterpret_cast<Unit<Base> *>(new INST_A64(TBZ_TBNZ)(INST_A64(TBZ_TBNZ)::TBZ, rt, bit, offset))));
}

void AssemblerA64::Tbnz(RegisterA64 &rt, U32 bit, Off offset) {
    Emit((reinterpret_cast<Unit<Base> *>(new INST_A64(TBZ_TBNZ)(INST_A64(TBZ_TBNZ)::TBNZ, rt, bit, offset))));
}

void AssemblerA64::Tbnz(RegisterA64 &rt, U32 bit, Label *label) {
    Emit((reinterpret_cast<Unit<Base> *>(new INST_A64(TBZ_TBNZ)(INST_A64(TBZ_TBNZ)::TBNZ, rt, bit, *label))));

}

void AssemblerA64::Cbz(RegisterA64 &rt, Off offset) {
    Emit((reinterpret_cast<Unit<Base> *>(new INST_A64(CBZ_CBNZ)(INST_A64(CBZ_CBNZ)::CBZ, offset, rt))));
}

void AssemblerA64::Cbz(RegisterA64 &rt, Label *label) {
    Emit((reinterpret_cast<Unit<Base> *>(new INST_A64(CBZ_CBNZ)(INST_A64(CBZ_CBNZ)::CBZ, *label, rt))));

}

void AssemblerA64::Cbnz(RegisterA64 &rt, Off offset) {
    Emit((reinterpret_cast<Unit<Base> *>(new INST_A64(CBZ_CBNZ)(INST_A64(CBZ_CBNZ)::CBNZ, offset, rt))));

}

void AssemblerA64::Cbnz(RegisterA64 &rt, Label *label) {
    Emit((reinterpret_cast<Unit<Base> *>(new INST_A64(CBZ_CBNZ)(INST_A64(CBZ_CBNZ)::CBNZ, *label, rt))));
}

void AssemblerA64::Str(RegisterA64 &rt, const MemOperand& memOperand) {
    if (memOperand.addr_mode == Offset) {
        Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(STR_UIMM)(rt, memOperand)));
    } else {
        Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(STR_IMM)(rt, memOperand)));
    }
}

void AssemblerA64::Ldr(RegisterA64 &rt, const MemOperand &memOperand) {
    if (memOperand.addr_mode == Offset) {
        Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(LDR_UIMM)(rt, memOperand)));
    } else {
        Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(LDR_IMM)(rt, memOperand)));
    }
}

void AssemblerA64::Ldr(RegisterA64 &rt, Label* label) {
    Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(LDR_LIT)(rt.isX() ? INST_A64(LDR_LIT)::LDR_X : INST_A64(LDR_LIT)::LDR_W, rt, *label)));
}

void AssemblerA64::Ldrsw(RegisterA64 &rt, Label* label) {
    Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(LDR_LIT)(INST_A64(LDR_LIT)::LDR_SW, rt, *label)));
}

void AssemblerA64::Ldrsw(XRegister &rt, const MemOperand& memOperand) {
    if (memOperand.addr_mode == Offset) {
        Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(LDRSW_UIMM)(rt, memOperand)));
    } else {
        Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(LDRSW_IMM)(rt, memOperand)));
    }
}

// If the current stack pointer is sp, then it must be aligned to 16 bytes
// on entry and the total size of the specified registers must also be a
// multiple of 16 bytes.
void AssemblerA64::Pop(RegisterA64 &rd) {
    if (rd.isX()) {
        Ldr(rd, MemOperand(&SP, 16, PostIndex));
    } else {
        Ldr(rd, MemOperand(&WSP, 16, PostIndex));
    }
}

// If the current stack pointer is sp, then it must be aligned to 16 bytes
// on entry and the total size of the specified registers must also be a
// multiple of 16 bytes.
void AssemblerA64::Push(RegisterA64 &rt) {
    if (rt.isX()) {
        Str(rt, MemOperand(&SP, -16, PreIndex));
    } else {
        Str(rt, MemOperand(&WSP, -16, PreIndex));
    }
}

void AssemblerA64::Cmp(RegisterA64 &rn, const Operand &operand) {
    Subs(*zeroRegFor(rn), rn, operand);
}

void AssemblerA64::Subs(RegisterA64 &rd, RegisterA64 &rn, const Operand &operand) {
    Emit(reinterpret_cast<Unit<Base> *>(new INST_A64(SUB_EXT_REG)(rd, rn, operand, SetFlags)));
}




