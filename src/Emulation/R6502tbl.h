#ifndef __6502TBL_H
#define __6502TBL_H

typedef void (R6502::*ImpliedCallProxy)();
typedef void (R6502::*OperCallProxy)( BytePair & );

enum EndInstructionResults
{
	LOAD_INSTRUCTION,
	DO_NMI,
	DO_IRQ
};

const OperCallProxy g_ArithOps[] = {
	&R6502::NOP, &R6502::ORA, &R6502::NOP, &R6502::SLO, &R6502::NOP, &R6502::ORA, &R6502::ASL, &R6502::SLO, 
	&R6502::NOP, &R6502::ORA, &R6502::ASL, &R6502::ANC, &R6502::NOP, &R6502::ORA, &R6502::ASL, &R6502::SLO,
	&R6502::NOP, &R6502::ORA, &R6502::NOP, &R6502::SLO, &R6502::NOP, &R6502::ORA, &R6502::ASL, &R6502::SLO, 
	&R6502::NOP, &R6502::ORA, &R6502::NOP, &R6502::SLO, &R6502::NOP, &R6502::ORA, &R6502::ASL, &R6502::SLO,
	&R6502::NOP, &R6502::AND, &R6502::NOP, &R6502::RLA, &R6502::BIT, &R6502::AND, &R6502::ROL, &R6502::RLA, 
	&R6502::NOP, &R6502::AND, &R6502::ROL, &R6502::ANC, &R6502::BIT, &R6502::AND, &R6502::ROL, &R6502::RLA,
	&R6502::NOP, &R6502::AND, &R6502::NOP, &R6502::RLA, &R6502::NOP, &R6502::AND, &R6502::ROL, &R6502::RLA, 
	&R6502::NOP, &R6502::AND, &R6502::NOP, &R6502::RLA, &R6502::NOP, &R6502::AND, &R6502::ROL, &R6502::RLA,
	&R6502::NOP, &R6502::EOR, &R6502::NOP, &R6502::SRE, &R6502::NOP, &R6502::EOR, &R6502::LSR, &R6502::SRE, 
	&R6502::NOP, &R6502::EOR, &R6502::LSR, &R6502::ASR, &R6502::NOP, &R6502::EOR, &R6502::LSR, &R6502::SRE,
	&R6502::NOP, &R6502::EOR, &R6502::NOP, &R6502::SRE, &R6502::NOP, &R6502::EOR, &R6502::LSR, &R6502::SRE, 
	&R6502::NOP, &R6502::EOR, &R6502::NOP, &R6502::SRE, &R6502::NOP, &R6502::EOR, &R6502::LSR, &R6502::SRE,
	&R6502::NOP, &R6502::ADC, &R6502::NOP, &R6502::RRA, &R6502::NOP, &R6502::ADC, &R6502::ROR, &R6502::RRA, 
	&R6502::NOP, &R6502::ADC, &R6502::ROR, &R6502::ARR, &R6502::NOP, &R6502::ADC, &R6502::ROR, &R6502::RRA,
	&R6502::NOP, &R6502::ADC, &R6502::NOP, &R6502::RRA, &R6502::NOP, &R6502::ADC, &R6502::ROR, &R6502::RRA, 
	&R6502::NOP, &R6502::ADC, &R6502::NOP, &R6502::RRA, &R6502::NOP, &R6502::ADC, &R6502::ROR, &R6502::RRA,
	&R6502::NOP, &R6502::STA, &R6502::NOP, &R6502::SAX, &R6502::STY, &R6502::STA, &R6502::STX, &R6502::SAX, 
	&R6502::NOP, &R6502::NOP, &R6502::NOP, &R6502::ANE, &R6502::STY, &R6502::STA, &R6502::STX, &R6502::SAX,
	&R6502::NOP, &R6502::STA, &R6502::NOP, &R6502::SHA, &R6502::STY, &R6502::STA, &R6502::STX, &R6502::SAX, 
	&R6502::NOP, &R6502::STA, &R6502::NOP, &R6502::SHS, &R6502::SHY, &R6502::STA, &R6502::SHX, &R6502::SHA,
	&R6502::LDY, &R6502::LDA, &R6502::LDX, &R6502::LAX, &R6502::LDY, &R6502::LDA, &R6502::LDX, &R6502::LAX, 
	&R6502::NOP, &R6502::LDA, &R6502::NOP, &R6502::LXA, &R6502::LDY, &R6502::LDA, &R6502::LDX, &R6502::LAX,
	&R6502::NOP, &R6502::LDA, &R6502::NOP, &R6502::LAX, &R6502::LDY, &R6502::LDA, &R6502::LDX, &R6502::LAX, 
	&R6502::NOP, &R6502::LDA, &R6502::NOP, &R6502::LAS, &R6502::LDY, &R6502::LDA, &R6502::LDX, &R6502::LAX,
	&R6502::CPY, &R6502::CMP, &R6502::NOP, &R6502::DCP, &R6502::CPY, &R6502::CMP, &R6502::DEC, &R6502::DCP, 
	&R6502::NOP, &R6502::CMP, &R6502::NOP, &R6502::SBX, &R6502::CPY, &R6502::CMP, &R6502::DEC, &R6502::DCP,
	&R6502::NOP, &R6502::CMP, &R6502::NOP, &R6502::DCP, &R6502::NOP, &R6502::CMP, &R6502::DEC, &R6502::DCP, 
	&R6502::NOP, &R6502::CMP, &R6502::NOP, &R6502::DCP, &R6502::NOP, &R6502::CMP, &R6502::DEC, &R6502::DCP,
	&R6502::CPX, &R6502::SBC, &R6502::NOP, &R6502::ISB, &R6502::CPX, &R6502::SBC, &R6502::INC, &R6502::ISB, 
	&R6502::NOP, &R6502::SBC, &R6502::NOP, &R6502::SBC, &R6502::CPX, &R6502::SBC, &R6502::INC, &R6502::ISB,
	&R6502::NOP, &R6502::SBC, &R6502::NOP, &R6502::ISB, &R6502::NOP, &R6502::SBC, &R6502::INC, &R6502::ISB, 
	&R6502::NOP, &R6502::SBC, &R6502::NOP, &R6502::ISB, &R6502::NOP, &R6502::SBC, &R6502::INC, &R6502::ISB
};

const ImpliedCallProxy g_MemAccess[] = {
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	NULL,
	NULL,
	NULL,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	NULL,
	NULL,
	NULL,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedWrite,
	NULL,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	NULL,
	NULL,
	NULL,
	NULL,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	NULL,
	&R6502::SharedWrite,
	NULL,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	NULL,
	&R6502::SharedWrite,
	NULL,
	&R6502::SharedRead,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	&R6502::SharedWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedRead,
	NULL,
	NULL,
	NULL,
	NULL,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	NULL,
	NULL,
	NULL,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	NULL,
	NULL,
	NULL,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
	NULL,
	&R6502::SharedRead,
	NULL,
	&R6502::SharedReadWrite,
	&R6502::SharedRead,
	&R6502::SharedRead,
	&R6502::SharedReadWrite,
	&R6502::SharedReadWrite,
};

const ImpliedCallProxy g_BaseCalls[] = {
	&R6502::Break,
	&R6502::IndexedIndirect,
	&R6502::Jam,
	&R6502::IndexedIndirect,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::PushP,
	&R6502::Immediate,
	&R6502::ASL_I,
	&R6502::Immediate,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Branch_NC,
	&R6502::IndirectIndexed_R,
	&R6502::Jam,
	&R6502::IndirectIndexed_W,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::CLC_I,
	&R6502::AbsoluteY_R,
	&R6502::NOP_I,
	&R6502::AbsoluteY_W,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteX_W,
	&R6502::AbsoluteX_W,
	&R6502::JumpSub,
	&R6502::IndexedIndirect,
	&R6502::Jam,
	&R6502::IndexedIndirect,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::PopP,
	&R6502::Immediate,
	&R6502::ROL_I,
	&R6502::Immediate,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Branch_NS,
	&R6502::IndirectIndexed_R,
	&R6502::Jam,
	&R6502::IndirectIndexed_W,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::SEC_I,
	&R6502::AbsoluteY_R,
	&R6502::NOP_I,
	&R6502::AbsoluteY_W,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteX_W,
	&R6502::AbsoluteX_W,
	&R6502::ReturnInterrupt,
	&R6502::IndexedIndirect,
	&R6502::Jam,
	&R6502::IndexedIndirect,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::PushA,
	&R6502::Immediate,
	&R6502::LSR_I,
	&R6502::Immediate,
	&R6502::Jump,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Branch_VC,
	&R6502::IndirectIndexed_R,
	&R6502::Jam,
	&R6502::IndirectIndexed_W,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::CLI_I,
	&R6502::AbsoluteY_R,
	&R6502::NOP_I,
	&R6502::AbsoluteY_W,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteX_W,
	&R6502::AbsoluteX_W,
	&R6502::ReturnSub,
	&R6502::IndexedIndirect,
	&R6502::Jam,
	&R6502::IndexedIndirect,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::PopA,
	&R6502::Immediate,
	&R6502::ROR_I,
	&R6502::Immediate,
	&R6502::JumpIndirect,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Branch_VS,
	&R6502::IndirectIndexed_R,
	&R6502::Jam,
	&R6502::IndirectIndexed_W,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::SEI_I,
	&R6502::AbsoluteY_R,
	&R6502::NOP_I,
	&R6502::AbsoluteY_W,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteX_W,
	&R6502::AbsoluteX_W,
	&R6502::Immediate,
	&R6502::IndexedIndirect,
	&R6502::Immediate,
	&R6502::IndexedIndirect,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::DEY_I,
	&R6502::Immediate,
	&R6502::TXA_I,
	&R6502::Immediate,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Branch_CC,
	&R6502::IndirectIndexed_W,
	&R6502::Jam,
	&R6502::IndirectIndexed_W,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageY,
	&R6502::ZeropageY,
	&R6502::TYA_I,
	&R6502::AbsoluteY_W,
	&R6502::TXS_I,
	&R6502::AbsoluteY_R,
	&R6502::AbsoluteX_W,
	&R6502::AbsoluteX_W,
	&R6502::AbsoluteY_W,
	&R6502::AbsoluteY_W,
	&R6502::Immediate,
	&R6502::IndexedIndirect,
	&R6502::Immediate,
	&R6502::IndexedIndirect,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::TAY_I,
	&R6502::Immediate,
	&R6502::TAX_I,
	&R6502::Immediate,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Branch_CS,
	&R6502::IndirectIndexed_R,
	&R6502::Jam,
	&R6502::IndirectIndexed_R,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageY,
	&R6502::ZeropageY,
	&R6502::CLV_I,
	&R6502::AbsoluteY_R,
	&R6502::TSX_I,
	&R6502::AbsoluteY_R,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteY_R,
	&R6502::AbsoluteY_R,
	&R6502::Immediate,
	&R6502::IndexedIndirect,
	&R6502::Immediate,
	&R6502::IndexedIndirect,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::INY_I,
	&R6502::Immediate,
	&R6502::DEX_I,
	&R6502::Immediate,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Branch_ZC,
	&R6502::IndirectIndexed_R,
	&R6502::Jam,
	&R6502::IndirectIndexed_W,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::CLD_I,
	&R6502::AbsoluteY_R,
	&R6502::NOP_I,
	&R6502::AbsoluteY_W,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteX_W,
	&R6502::AbsoluteX_W,
	&R6502::Immediate,
	&R6502::IndexedIndirect,
	&R6502::Immediate,
	&R6502::IndexedIndirect,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::Zeropage,
	&R6502::INX_I,
	&R6502::Immediate,
	&R6502::NOP_I,
	&R6502::Immediate,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Absolute,
	&R6502::Branch_ZS,
	&R6502::IndirectIndexed_R,
	&R6502::Jam,
	&R6502::IndirectIndexed_W,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::ZeropageX,
	&R6502::SED_I,
	&R6502::AbsoluteY_R,
	&R6502::NOP_I,
	&R6502::AbsoluteY_W,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteX_R,
	&R6502::AbsoluteX_W,
	&R6502::AbsoluteX_W
};

#endif