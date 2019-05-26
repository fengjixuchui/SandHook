//
// Created by swift on 2019/5/16.
//

#include "inst_t16.h"
#include "arm32_base.h"
#include "register_list_arm32.h"

#define SET_BASE_OPCODE(X) get()->opcode_base = OPCODE_T16(X)

#define SET_OPCODE(X) get()->opcode = OPCODE_T16(X)
#define SET_OPCODE_MULTI(X, INDEX) get()->opcode##INDEX = OPCODE_T16(X##_##INDEX)

using namespace SandHook::AsmA32;
using namespace SandHook::RegistersA32;


//Unknow

T16_UNKNOW::T16_UNKNOW(STRUCT_T16(UNKNOW) &inst) : InstructionT16(&inst) {
    decode(&inst);
}

void T16_UNKNOW::decode(T16_STRUCT_UNKNOW *inst) {
    inst_backup = *inst;
}

void T16_UNKNOW::assembler() {
    set(inst_backup);
}

//B
T16_B::T16_B() {}

T16_B::T16_B(T16_STRUCT_B *inst) : T16_INST_PC_REL(inst) {
    decode(inst);
}

T16_B::T16_B(Off offset) : offset(offset) {}

T16_B::T16_B(Label &label) {
    bindLabel(label);
}

Off T16_B::getImmPCOffset() {
    return DECODE_OFFSET(11, 1);
}

void T16_B::decode(T16_STRUCT_B *inst) {
    offset = getImmPCOffset();
}

void T16_B::assembler() {
    SET_OPCODE(B);
    DECODE_OFFSET(11, 1);
}

void T16_B::onOffsetApply(Off offset) {
    this->offset = offset;
    DECODE_OFFSET(11, 1);
}



//B Cond
T16_B_COND::T16_B_COND() {}

T16_B_COND::T16_B_COND(STRUCT_T16(B_COND) *inst) : T16_INST_PC_REL(inst) {
    decode(inst);
}

T16_B_COND::T16_B_COND(Condition condition, Off offset) : condition(condition), offset(offset) {}

T16_B_COND::T16_B_COND(Condition condition, Label &label) {
    bindLabel(label);
}

void T16_B_COND::decode(STRUCT_T16(B_COND) *inst) {
    DECODE_COND;
    offset = getImmPCOffset();
}

void T16_B_COND::assembler() {
    SET_OPCODE(B_COND);
    ENCODE_COND;
    ENCODE_OFFSET(8, 1);
}

void T16_B_COND::onOffsetApply(Off offset) {
    this->offset = offset;
    ENCODE_OFFSET(8, 1);
}

Off T16_B_COND::getImmPCOffset() {
    return DECODE_OFFSET(8, 1);
}



//BX BLX
T16_BX_BLX::T16_BX_BLX(T16_BX_BLX::OP op, RegisterA32 &rm) : op(op), rm(&rm) {}

T16_BX_BLX::T16_BX_BLX(T16_STRUCT_BX_BLX *inst) : T16_INST_PC_REL(inst) {
    decode(inst);
}

void T16_BX_BLX::decode(STRUCT_T16(BX_BLX) *inst) {
    DECODE_OP;
    DECODE_RM(Reg);
}

void T16_BX_BLX::assembler() {
    SET_OPCODE_MULTI(BX_BLX, 1);
    SET_OPCODE_MULTI(BX_BLX, 2);
    ENCODE_OP;
    ENCODE_RM;
}


//CBZ CBNZ
T16_CBZ_CBNZ::T16_CBZ_CBNZ(T16_STRUCT_CBZ_CBNZ *inst) : T16_INST_PC_REL(inst) {
    decode(inst);
}

T16_CBZ_CBNZ::T16_CBZ_CBNZ(T16_CBZ_CBNZ::OP op, Off offset, RegisterA32 &rn) : op(op), offset(offset),
                                                                              rn(&rn) {}


T16_CBZ_CBNZ::T16_CBZ_CBNZ(T16_CBZ_CBNZ::OP op, Label& label, RegisterA32 &rn) : op(op),
                                                                               rn(&rn) {
    bindLabel(label);
}



Off T16_CBZ_CBNZ::getImmPCOffset() {
    return COMBINE(get()->i, get()->imm5, 5) << 2;
}

void T16_CBZ_CBNZ::decode(T16_STRUCT_CBZ_CBNZ *inst) {
    offset = getImmPCOffset();
    DECODE_RN(Reg);
    DECODE_OP;
}

void T16_CBZ_CBNZ::assembler() {
    SET_OPCODE_MULTI(CBZ_CBNZ, 1);
    SET_OPCODE_MULTI(CBZ_CBNZ, 2);
    SET_OPCODE_MULTI(CBZ_CBNZ, 3);
    ENCODE_OP;
    ENCODE_RN;
    ENCODE_OFFSET(5, 2);
}

void T16_CBZ_CBNZ::onOffsetApply(Off offset) {
    this->offset = offset;
    ENCODE_OFFSET(5, 2);
}


//LDR_LIT
T16_LDR_LIT::T16_LDR_LIT(T16_STRUCT_LDR_LIT *inst) : T16_INST_PC_REL(inst) {
    decode(inst);
}

T16_LDR_LIT::T16_LDR_LIT(Off offset, RegisterA32 &rt) : offset(offset), rt(&rt) {

}

Off T16_LDR_LIT::getImmPCOffset() {
    return get()->imm8 << 2;
}

Addr T16_LDR_LIT::getImmPCOffsetTarget() {
    return ALIGN((Addr) getPC() + offset, 4);
}

void T16_LDR_LIT::onOffsetApply(Off offset) {
    this->offset = offset;
    ENCODE_OFFSET(8, 2);
}

void T16_LDR_LIT::decode(T16_STRUCT_LDR_LIT *inst) {
    DECODE_RT(Reg);
    offset = getImmPCOffset();
}

void T16_LDR_LIT::assembler() {
    SET_OPCODE(LDR_LIT);
    ENCODE_RT;
    ENCODE_OFFSET(8, 2);
}


//ADD IMM RD = RN
T16_ADD_IMM_RDN::T16_ADD_IMM_RDN(T16_STRUCT_ADD_IMM_RDN *inst) : InstructionT16(inst) {
    decode(inst);
}


T16_ADD_IMM_RDN::T16_ADD_IMM_RDN(RegisterA32 *rdn, U8 imm8) : rdn(rdn), imm8(imm8) {}


void T16_ADD_IMM_RDN::decode(T16_STRUCT_ADD_IMM_RDN *inst) {
    rdn = Reg(inst->rdn);
    imm8 = inst->imm8;
}

void T16_ADD_IMM_RDN::assembler() {
    SET_OPCODE(ADD_IMM_RDN);
    get()->imm8 = imm8;
    get()->rdn = rdn->getCode();
}




//ADR
T16_ADR::T16_ADR(T16_STRUCT_ADR *inst) : T16_INST_PC_REL(inst) {
    decode(inst);
}

T16_ADR::T16_ADR(RegisterA32 &rd, Off offset) : rd(&rd), offset(offset) {}

T16_ADR::T16_ADR(RegisterA32 &rd, Label &label) : rd(&rd) {
    bindLabel(label);
}


Off T16_ADR::getImmPCOffset() {
    return COMBINE(get()->imm8, 0 ,2);
}

Addr T16_ADR::getImmPCOffsetTarget() {
    return RoundDown((Addr) getPC() + offset, 4);
}

void T16_ADR::onOffsetApply(Off offset) {
    this->offset = offset;
    get()->imm8 = (U32)offset >> 2;
}

void T16_ADR::decode(T16_STRUCT_ADR *inst) {
    offset = getImmPCOffset();
    DECODE_RD(Reg);
}


void T16_ADR::assembler() {
    SET_OPCODE(ADR);
    ENCODE_RD;
    get()->imm8 = (U32)offset >> 2;
}

T16_CMP_REG::T16_CMP_REG() {}

T16_CMP_REG::T16_CMP_REG(T16_STRUCT_CMP_REG *inst) : InstructionT16(inst) {
    decode(inst);
}


T16_CMP_REG::T16_CMP_REG(RegisterA32 &rm, RegisterA32 &rn) : rm(&rm), rn(&rn) {}

void T16_CMP_REG::decode(T16_STRUCT_CMP_REG *inst) {
    DECODE_RM(Reg);
    DECODE_RN(Reg);
}

void T16_CMP_REG::assembler() {
    SET_BASE_OPCODE(DATA_PROC);
    SET_OPCODE(CMP_REG);
    ENCODE_RM;
    ENCODE_RN;
}



//MOV REG
T16_MOV_REG::T16_MOV_REG(T16_STRUCT_MOV_REG *inst) : InstructionT16(inst) {
    decode(inst);
}


T16_MOV_REG::T16_MOV_REG(RegisterA32 &rd, RegisterA32 &rm) : rm(&rm), rd(&rd) {
}

void T16_MOV_REG::decode(T16_STRUCT_MOV_REG *inst) {
    DECODE_RM(Reg);
    rd = Reg(static_cast<U8>(COMBINE(inst->D, inst->rd, 3)));
}

void T16_MOV_REG::assembler() {
    SET_BASE_OPCODE(DATA_PROC);
    SET_OPCODE(MOV_REG);
    ENCODE_RM;
    get()->rd = BITS(rd->getCode(), 0, 2);
    get()->D = BIT(rd->getCode(), 3);
}

bool T16_MOV_REG::pcRelate() {
    return rd == &PC || rm == &PC;
}



T16_ADD_REG::T16_ADD_REG() {}

T16_ADD_REG::T16_ADD_REG(T16_STRUCT_ADD_REG *inst) : InstructionT16(inst) {
    decode(inst);
}

T16_ADD_REG::T16_ADD_REG(RegisterA32 *rd, RegisterA32 *rn, RegisterA32 *rm) : rd(rd), rn(rn),
                                                                              rm(rm) {}

void T16_ADD_REG::decode(T16_STRUCT_ADD_REG *inst) {
    DECODE_RD(Reg);
    DECODE_RN(Reg);
    DECODE_RM(Reg);
}

void T16_ADD_REG::assembler() {
    SET_OPCODE(ADD_REG);
    INST_ASSERT(rd->getCode() > 7);
    INST_ASSERT(rn->getCode() > 7);
    INST_ASSERT(rm->getCode() > 7);
    ENCODE_RD;
    ENCODE_RN;
    ENCODE_RM;
}



T16_CMP_REG_EXT::T16_CMP_REG_EXT(T16_STRUCT_CMP_REG_EXT *inst) : InstructionT16(inst) {
    decode(inst);
}

T16_CMP_REG_EXT::T16_CMP_REG_EXT(RegisterA32 &rn, RegisterA32 &rm) : rn(&rn), rm(&rm) {}

void T16_CMP_REG_EXT::decode(T16_STRUCT_CMP_REG_EXT *inst) {
    rn = Reg(COMBINE(inst->N, inst->rn, 3));
    DECODE_RM(Reg);
}

void T16_CMP_REG_EXT::assembler() {
    SET_OPCODE(CMP_REG_EXT);
    ENCODE_RM;
    get()->rn = BITS(rn->getCode(), 0, 2);
    get()->N = BIT(rn->getCode(), 3);
}


//POP
T16_POP::T16_POP(T16_STRUCT_POP *inst) : InstructionT16(inst) {
    decode(inst);
}

T16_POP::T16_POP(const RegisterList &registerList) : registerList(registerList) {}

void T16_POP::decode(T16_STRUCT_POP *inst) {
    registerList.SetList(COMBINE(inst->P << 7, inst->regs, 8));
}

void T16_POP::assembler() {
    SET_OPCODE(POP);
    U16 regs = registerList.GetList();
    get()->regs = BITS(regs, 0, 7);
    get()->P = BIT(regs, 15);
}


//PUSH
T16_PUSH::T16_PUSH(T16_STRUCT_PUSH *inst) : InstructionT16(inst) {
    decode(inst);
}

T16_PUSH::T16_PUSH(const RegisterList &registerList) : registerList(registerList) {}

void T16_PUSH::decode(T16_STRUCT_PUSH *inst) {
    registerList.SetList(COMBINE(inst->M << 6, inst->regs, 8));
}

void T16_PUSH::assembler() {
    SET_OPCODE(PUSH);
    U16 regs = registerList.GetList();
    get()->regs = BITS(regs, 0, 7);
    get()->M = BIT(regs, 14);
}


// Add reg rdn T2
T16_ADD_REG_RDN::T16_ADD_REG_RDN(T16_STRUCT_ADD_REG_RDN *inst) : InstructionT16(inst) {
    decode(inst);
}

T16_ADD_REG_RDN::T16_ADD_REG_RDN(RegisterA32 &rdn, RegisterA32 &rm) : rdn(&rdn), rm(&rm) {}

bool T16_ADD_REG_RDN::pcRelate() {
    return *rm == PC;
}

void T16_ADD_REG_RDN::decode(T16_STRUCT_ADD_REG_RDN *inst) {
    DECODE_RM(Reg);
    rdn = Reg(inst->rdn);
}

void T16_ADD_REG_RDN::assembler() {
    SET_OPCODE(ADD_REG_RDN);
    ENCODE_RM;
    get()->rdn = rdn->getCode();
}
