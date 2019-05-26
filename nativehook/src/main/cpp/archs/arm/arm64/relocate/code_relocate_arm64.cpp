//
// Created by swift on 2019/5/12.
//

#include "code_relocate_arm64.h"
#include "decoder.h"
#include "lock.h"

using namespace SandHook::Decoder;
using namespace SandHook::RegistersA64;
using namespace SandHook::AsmA64;
using namespace SandHook::Utils;

#define __ assemblerA64->

#define IMPL_RELOCATE(X) void CodeRelocateA64::relocate_##X (INST_A64(X)* inst, void* toPc) throw(ErrorCodeException)

#define CASE(X) \
case ENUM_VALUE(InstCodeA64, InstCodeA64::X): \
relocate_##X(reinterpret_cast<INST_A64(X)*>(instruction), toPc); \
break;

CodeRelocateA64::CodeRelocateA64(AssemblerA64 &assembler) : CodeRelocate(assembler.codeContainer) {
    assemblerA64 = &assembler;
}

bool CodeRelocateA64::visit(Unit<Base> *unit, void *pc) {
    relocate(reinterpret_cast<Instruction<Base> *>(unit), __ getPC());
    curOffset += unit->size();
    if (unit->refcount() == 0) {
        delete unit;
    }
    return true;
}

void* CodeRelocateA64::relocate(void *startPc, Addr len, void *toPc = nullptr) throw(ErrorCodeException) {
    AutoLock autoLock(relocateLock);
    startAddr = reinterpret_cast<Addr>(startPc);
    length = len;
    curOffset = 0;
    __ allocBufferFirst(static_cast<U32>(len * 8));
    void* curPc = __ getPC();
    if (toPc == nullptr) {
        Disassembler::get()->decode(startPc, len, *this, true);
    } else {
        //TODO
    }
    return curPc;
}

void* CodeRelocateA64::relocate(Instruction<Base> *instruction, void *toPc) throw(ErrorCodeException) {
    void* curPc = __ getPC();

    //insert later bind labels
    __ Emit(getLaterBindLabel(curOffset));

    if (!instruction->pcRelate()) {
        __ Emit(instruction);
        instruction->ref();
        return curPc;
    }
    switch (instruction->instCode()) {
        CASE(B_BL)
        CASE(B_COND)
        CASE(TBZ_TBNZ)
        CASE(CBZ_CBNZ)
        CASE(LDR_LIT)
        CASE(ADR_ADRP)
        default:
            __ Emit(instruction);
            instruction->ref();
    }
    return curPc;
}

IMPL_RELOCATE(B_BL) {

    if (inRelocateRange(inst->offset, sizeof(InstA64))) {
        inst->ref();
        inst->bindLabel(*getLaterBindLabel(inst->offset + curOffset));
        __ Emit(reinterpret_cast<Instruction<Base>*>(inst));
        return;
    }

    Addr targetAddr = inst->getImmPCOffsetTarget();

    if (inst->op == inst->BL) {
        Addr lr = reinterpret_cast<Addr>(toPc);
        lr += 4 * 4; // MovWide * 4;
        lr += 4 * 4; // MovWide * 4;
        lr += 4; // Br
        __ Mov(LR, lr);
    }
    __ Mov(IP1, targetAddr);
    __ Br(IP1);
}

IMPL_RELOCATE(B_COND) {

    if (inRelocateRange(inst->offset, sizeof(InstA64))) {
        inst->ref();
        inst->bindLabel(*getLaterBindLabel(inst->offset + curOffset));
        __ Emit(reinterpret_cast<Instruction<Base>*>(inst));
        return;
    }

    Addr targetAddr = inst->getImmPCOffsetTarget();

    Label *true_label = new Label();
    Label *false_label = new Label();

    __ B(inst->condition, true_label);
    __ B(false_label);

    __ Emit(true_label);
    __ Mov(IP1, targetAddr);
    __ Br(IP1);

    __ Emit(false_label);
}

IMPL_RELOCATE(TBZ_TBNZ) {

    if (inRelocateRange(inst->offset, sizeof(InstA64))) {
        inst->ref();
        inst->bindLabel(*getLaterBindLabel(inst->offset + curOffset));
        __ Emit(reinterpret_cast<Instruction<Base>*>(inst));
        return;
    }

    Addr targetAddr = inst->getImmPCOffsetTarget();

    Label *true_label = new Label();
    Label *false_label = new Label();

    if (inst->op == INST_A64(TBZ_TBNZ)::TBNZ) {
        __ Tbnz(*inst->rt, inst->bit, true_label);
    } else {
        __ Tbz(*inst->rt, inst->bit, true_label);
    }
    __ B(false_label);

    __ Emit(true_label);
    __ Mov(IP1, targetAddr);
    __ Br(IP1);

    __ Emit(false_label);
}

IMPL_RELOCATE(CBZ_CBNZ) {

    if (inRelocateRange(inst->offset, sizeof(InstA64))) {
        inst->ref();
        inst->bindLabel(*getLaterBindLabel(inst->offset + curOffset));
        __ Emit(reinterpret_cast<Instruction<Base>*>(inst));
        return;
    }

    Addr targetAddr = inst->getImmPCOffsetTarget();

    Label *true_label = new Label();
    Label *false_label = new Label();

    if (inst->op == INST_A64(CBZ_CBNZ)::CBNZ) {
        __ Cbnz(*inst->rt, true_label);
    } else {
        __ Cbz(*inst->rt, true_label);
    }

    __ B(false_label);

    __ Emit(true_label);
    __ Mov(IP1, targetAddr);
    __ Br(IP1);

    __ Emit(false_label);
}

IMPL_RELOCATE(LDR_LIT) {
    Addr targetAddr = inst->getImmPCOffsetTarget();
    XRegister* rtX = XReg(inst->rt->getCode());
    WRegister* rtW = WReg(inst->rt->getCode());

    if (inRelocateRange(inst->offset, sizeof(Addr))) {
        inst->ref();
        inst->bindLabel(*getLaterBindLabel(inst->offset + curOffset));
        __ Emit(reinterpret_cast<Instruction<Base>*>(inst));
        return;
    }

    switch (inst->op) {
        case INST_A64(LDR_LIT)::LDR_X:
            __ Mov(*rtX, targetAddr);
            __ Ldr(*rtX, MemOperand(rtX, 0, Offset));
            break;
        case INST_A64(LDR_LIT)::LDR_W:
            __ Mov(*rtX, targetAddr);
            __ Ldr(*rtW, MemOperand(rtX, 0, Offset));
            break;
        case INST_A64(LDR_LIT)::LDR_SW:
            __ Mov(*rtX, targetAddr);
            __ Ldrsw(*rtX, MemOperand(rtX, 0, Offset));
            break;
        case INST_A64(LDR_LIT)::LDR_PRFM:
            __ Push(X0);
            __ Mov(X0, targetAddr);
            __ Ldrsw(X0, MemOperand(rtX, 0, Offset));
            __ Pop(X0);
            break;
    }
}

IMPL_RELOCATE(ADR_ADRP) {
    __ Mov(*inst->rd, inst->getImmPCOffsetTarget());
}
