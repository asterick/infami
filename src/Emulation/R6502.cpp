#include "Emulation.h"
#include "R6502tbl.h"

R6502::R6502()
{
	Stack.Word = 0x0100;	// Stack is always at 0x100~0x1FF
}

void R6502::Restart()
{
	m_EndInstructionResult	= LOAD_INSTRUCTION;
	m_SpriteDMA				= false;	
	m_NMIAck				= false;

	Reset();
}

void R6502::SpriteDMA( unsigned char Base )
{
	SprDma.Units.Base = Base;
	m_SpriteDMA = true;
}

inline unsigned char R6502::CompileP( bool Break )
{
	return 0x20 | // The "always on" flag
		( Carry			? 0x01 : 00 ) |
		( Zero			? 0x02 : 00 ) |
		( IRQDisable	? 0x04 : 00 ) |
		( Decimal		? 0x08 : 00 ) |
		( Break			? 0x10 : 00 ) |		
		( Overflow		? 0x40 : 00 ) |
		( Negitive		? 0x80 : 00 );
}

inline void R6502::DecompileP( unsigned char p )
{
	Carry		= (p & 0x01) != 0;
	Zero		= (p & 0x02) != 0;
	IRQDisable	= (p & 0x04) != 0;
	Decimal		= (p & 0x08) != 0;
	Overflow	= (p & 0x40) != 0;
	Negitive	= (p & 0x80) != 0;
}

inline void R6502::ClockHalt()
{
	while( HaltActive() )
		Clock();
	Clock();
}

void R6502::EndInstruction()
{	
	bool NMI = NMIActive();

	if( NMI && !m_NMIAck )	// Only fire if NMI has just been asserted
		m_EndInstructionResult = DO_NMI;
	else if( IRQActive() && !IRQDisable )
		m_EndInstructionResult = DO_IRQ;
	else
		m_EndInstructionResult = LOAD_INSTRUCTION;
	
	m_NMIAck     = NMI;
}

void R6502::CPUStep()
{
	if( m_SpriteDMA )
	{		
		m_SpriteDMA = false;
		SpriteDMA();
	}
	else if( m_EndInstructionResult == DO_NMI )
	{
		NonMaskInterrupt();
	}
	else if( m_EndInstructionResult == DO_IRQ )
	{
		MaskableInterrupt();
	}
	else
	{
		ClockHalt();
		m_OpCode = Read(PC.Word++);
		(this->*g_BaseCalls[m_OpCode])();
	}
}

void R6502::Reset()
{
	ClockHalt();
	Read( PC.Word );

	ClockHalt();
	Read( PC.Word );
	SprDma.Address = 0;		// Clear out DMA address

	Clock();
	Write( Stack.Word, PC.Unsigned.ByteH );
	Stack.Unsigned.ByteL--;

	Clock();
	Write( Stack.Word, PC.Unsigned.ByteL );
	Stack.Unsigned.ByteL--;

	ClockHalt();
	PC.Unsigned.ByteL = Read(0xFFFC);

	ClockHalt();
	PC.Unsigned.ByteH = Read(0xFFFD);
	EndInstruction();
}

void R6502::NonMaskInterrupt()
{
	Read(PC.Word);

	ClockHalt();
	Read( PC.Word );
	unsigned char P = CompileP(false);

	Clock();
	Write( Stack.Word, PC.Unsigned.ByteH );
	Stack.Unsigned.ByteL--;

	Clock();
	Write( Stack.Word, PC.Unsigned.ByteL );
	Stack.Unsigned.ByteL--;

	Clock();
	Write( Stack.Word, P );
	Stack.Unsigned.ByteL--;

	ClockHalt();
	PC.Unsigned.ByteL = Read(0xFFFA);

	ClockHalt();
	PC.Unsigned.ByteH = Read(0xFFFB);		
	EndInstruction();
}

void R6502::MaskableInterrupt()
{
	/* --- Interrupt / Break Handler --- */
	ClockHalt();
	Read(PC.Word);

	ClockHalt();
	Read( PC.Word );
	unsigned char P = CompileP(false);

	Clock();
	Write( Stack.Word, PC.Unsigned.ByteH );
	Stack.Unsigned.ByteL--;

	Clock();
	Write( Stack.Word, PC.Unsigned.ByteL );
	Stack.Unsigned.ByteL--;
	
	Clock();
	Write( Stack.Word, P );
	Stack.Unsigned.ByteL--;

	ClockHalt();
	IRQDisable = true;
	PC.Unsigned.ByteL = Read(0xFFFE);

	ClockHalt();
	PC.Unsigned.ByteH = Read(0xFFFF);
	EndInstruction();
}

void R6502::Break()
{
	ClockHalt();
	Read( PC.Word++ );
	unsigned char P = CompileP(true);

	Clock();
	Write( Stack.Word, PC.Unsigned.ByteH );
	Stack.Unsigned.ByteL--;

	Clock();
	Write( Stack.Word, PC.Unsigned.ByteL );
	Stack.Unsigned.ByteL--;

	Clock();
	Write( Stack.Word, P );
	Stack.Unsigned.ByteL--;

	ClockHalt();
	IRQDisable = true;
	PC.Unsigned.ByteL = Read(0xFFFE);

	ClockHalt();
	PC.Unsigned.ByteH = Read(0xFFFF);
	EndInstruction();
}

void R6502::SharedRead()
{
	ClockHalt();
	Arguement.Unsigned = Read(Effective.Word);
	EndInstruction();
	(this->*g_ArithOps[m_OpCode])(Arguement);
}

void R6502::SharedReadWrite()
{
	ClockHalt();
	Arguement.Unsigned = Read(Effective.Word);

	Clock();
	Write(Effective.Word,Arguement.Unsigned);

	Clock();
	(this->*g_ArithOps[m_OpCode])(Arguement);
	Write(Effective.Word,Arguement.Unsigned);
	EndInstruction();
}

void R6502::SharedWrite()
{
	Clock();
	(this->*g_ArithOps[m_OpCode])(Arguement);
	Write(Effective.Word,Arguement.Unsigned);
	EndInstruction();
}

void R6502::Immediate()
{
	Effective.Word = PC.Word++;
	SharedRead();
}

void R6502::Absolute()
{
	ClockHalt();
	Effective.Unsigned.ByteL = Read(PC.Word++);

	ClockHalt();
	Effective.Unsigned.ByteH = Read(PC.Word++);

	(this->*g_MemAccess[m_OpCode])();
}

void R6502::Zeropage()
{
	ClockHalt();
	Effective.Word = Read(PC.Word++);

	(this->*g_MemAccess[m_OpCode])();
}

void R6502::AbsoluteX_R()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);

	ClockHalt();
	Working.Unsigned.ByteH = Read(PC.Word++);
	Effective.Word = Working.Word + X;
	Working.Unsigned.ByteL = Effective.Unsigned.ByteL;

	if( Working.Word != Effective.Word )
	{
		ClockHalt();
		Read(Working.Word);
	}

	(this->*g_MemAccess[m_OpCode])();
}

void R6502::AbsoluteX_W()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);
	
	ClockHalt();
	Working.Unsigned.ByteH = Read(PC.Word++);
	Effective.Word = Working.Word + X;
	Working.Unsigned.ByteL = Effective.Unsigned.ByteL;

	ClockHalt();
	Read(Working.Word);

	(this->*g_MemAccess[m_OpCode])();
}

void R6502::AbsoluteY_R()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);

	ClockHalt();

	Working.Unsigned.ByteH = Read(PC.Word++);
	Effective.Word = Working.Word + Y;
	Working.Unsigned.ByteL = Effective.Unsigned.ByteL;

	if( Working.Word != Effective.Word )
	{
		ClockHalt();
		Read(Working.Word);
	}

	(this->*g_MemAccess[m_OpCode])();
}

void R6502::AbsoluteY_W()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);

	ClockHalt();
	Working.Unsigned.ByteH = Read(PC.Word++);
	Effective.Word = Working.Word + Y;
	Working.Unsigned.ByteL = Effective.Unsigned.ByteL;

	ClockHalt();
	Read(Working.Word);

	(this->*g_MemAccess[m_OpCode])();
}

void R6502::ZeropageX()
{
	ClockHalt();
	Effective.Word = Read(PC.Word++);

	ClockHalt();
	Read( Effective.Word );
	Effective.Unsigned.ByteL += X;
		
	(this->*g_MemAccess[m_OpCode])();
}

void R6502::ZeropageY()
{
	ClockHalt();
	Effective.Word = Read(PC.Word++);

	ClockHalt();
	Read( Effective.Word );
	Effective.Unsigned.ByteL += Y;
		
	(this->*g_MemAccess[m_OpCode])();
}

void R6502::IndexedIndirect()
{
	ClockHalt();
	Working.Word = Read(PC.Word++);

	ClockHalt();
	Read(Working.Word);
	Working.Unsigned.ByteL += X;

	ClockHalt();
	Effective.Unsigned.ByteL = Read(Working.Word);
	Working.Unsigned.ByteL++;

	ClockHalt();
	Effective.Unsigned.ByteH = Read(Working.Word);

	(this->*g_MemAccess[m_OpCode])();
}

void R6502::IndirectIndexed_R()
{
	ClockHalt();
	Effective.Word = Read(PC.Word++);

	ClockHalt();
	Working.Unsigned.ByteL = Read(Effective.Word);
	Effective.Unsigned.ByteL++;

	ClockHalt();
	Working.Unsigned.ByteH = Read(Effective.Word);
	Effective.Word = Working.Word + Y;
	Working.Unsigned.ByteL = Effective.Unsigned.ByteL;

	if( Working.Word != Effective.Word )
	{
		ClockHalt();
		Read(Working.Word);
	}

	(this->*g_MemAccess[m_OpCode])();
}

void R6502::IndirectIndexed_W()
{
	ClockHalt();
	Effective.Word = Read(PC.Word++);

	ClockHalt();
	Working.Unsigned.ByteL = Read(Effective.Word);
	Effective.Unsigned.ByteL++;

	ClockHalt();
	Working.Unsigned.ByteH = Read(Effective.Word);
	Effective.Word = Working.Word + Y;
	Working.Unsigned.ByteL = Effective.Unsigned.ByteL;

	ClockHalt();
	Read(Working.Word);
	
	(this->*g_MemAccess[m_OpCode])();
}

void R6502::SpriteDMA()
{	
	do
	{
		ClockHalt();
		Arguement.Unsigned = Read(SprDma.Address);

		Clock();
		Write(APU_SPR_DMA_TARGET,Arguement.Unsigned);
	} while(++SprDma.Units.Bytes != 0 );

	EndInstruction();
}

void R6502::Branch()
{
	ClockHalt();		
	Working.Word = PC.Word + Working.Signed.ByteL;
	PC.Unsigned.ByteL = Working.Unsigned.ByteL;
	Read(PC.Word);	// Useless fake read

	if( PC.Word != Working.Word )
	{
		ClockHalt();
		PC.Word = Working.Word;
		Read(PC.Word);	// Useless fake read
	}
}

void R6502::Branch_NC()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);

	if( !Negitive )
		Branch();

	EndInstruction();
}

void R6502::Branch_NS()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);
	
	if( Negitive )
		Branch();

	EndInstruction();
}

void R6502::Branch_VC()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);

	if( !Overflow )
		Branch();

	EndInstruction();
}

void R6502::Branch_VS()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);

	if( Overflow )
		Branch();

	EndInstruction();
}

void R6502::Branch_CC()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);

	if( !Carry )
		Branch();

	EndInstruction();
}

void R6502::Branch_CS()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);

	if( Carry )
		Branch();

	EndInstruction();
}

void R6502::Branch_ZC()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);

	if( !Zero )
		Branch();

	EndInstruction();
}

void R6502::Branch_ZS()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);
	
	if( Zero )
		Branch();

	EndInstruction();
}

void R6502::JumpSub()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);

	ClockHalt();
	Read(Stack.Word);

	Clock();
	Write(Stack.Word, PC.Unsigned.ByteH);
	Stack.Unsigned.ByteL--;

	Clock();
	Write(Stack.Word, PC.Unsigned.ByteL);
	Stack.Unsigned.ByteL--;

	ClockHalt();
	Working.Unsigned.ByteH = Read(PC.Word);
	EndInstruction();
	PC.Word = Working.Word;
}

void R6502::Jump()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);

	ClockHalt();
	Working.Unsigned.ByteH = Read(PC.Word);
	EndInstruction();
	PC.Word = Working.Word;
}

void R6502::JumpIndirect()
{
	ClockHalt();
	Working.Unsigned.ByteL = Read(PC.Word++);

	ClockHalt();
	Working.Unsigned.ByteH = Read(PC.Word++);

	ClockHalt();
	PC.Unsigned.ByteL = Read(Working.Word);
	Working.Unsigned.ByteL++;	// Do not load across pages

	ClockHalt();
	PC.Unsigned.ByteH = Read(Working.Word);
	EndInstruction();
}

void R6502::ReturnInterrupt()
{
	ClockHalt();
	Read( PC.Word );

	ClockHalt();
	Read(Stack.Word);
	Stack.Unsigned.ByteL++;

	ClockHalt();
	DecompileP( Read(Stack.Word) );
	Stack.Unsigned.ByteL++;

	ClockHalt();
	PC.Unsigned.ByteL = Read(Stack.Word);
	Stack.Unsigned.ByteL++;

	ClockHalt();
	PC.Unsigned.ByteH = Read(Stack.Word);
	EndInstruction();
}

void R6502::ReturnSub()
{
	ClockHalt();
	Read( PC.Word );

	ClockHalt();
	Read(Stack.Word);
	Stack.Unsigned.ByteL++;

	ClockHalt();
	PC.Unsigned.ByteL = Read(Stack.Word);
	Stack.Unsigned.ByteL++;

	ClockHalt();
	PC.Unsigned.ByteH = Read(Stack.Word);

	ClockHalt();
	Read( PC.Word );
	EndInstruction();
	PC.Word++;
}

void R6502::PushA()
{
	ClockHalt();
	Read(PC.Word);
	Working.Unsigned.ByteL = A;

	Clock();
	Write( Stack.Word, Working.Unsigned.ByteL );
	Stack.Unsigned.ByteL--;
	EndInstruction();
}

void R6502::PushP()
{
	ClockHalt();
	Read(PC.Word);
	unsigned char P = CompileP(true);

	Clock();
	Write( Stack.Word, P );
	Stack.Unsigned.ByteL--;
	EndInstruction();
}

void R6502::PopA()
{
	ClockHalt();
	Read(PC.Word);

	ClockHalt();
	Read(Stack.Word);
	Stack.Unsigned.ByteL++;

	ClockHalt();
	A = Read(Stack.Word);
	Zero = (A == 0);
	Negitive = ((A & 0x80) != 0);
	EndInstruction();
}

void R6502::PopP()
{
	ClockHalt();
	Read(PC.Word);

	ClockHalt();
	Read(Stack.Word);
	Stack.Unsigned.ByteL++;

	ClockHalt();
	EndInstruction();
	DecompileP( Read(Stack.Word) );
}

void R6502::Jam()
{
	printf( "CPU Jammed... Auto-rebooting.\n" );
	Restart();	
	
	// TODO: ACTUALLY JAM THE CPU
	//ClockHalt();
	//Read(PC.Word);
}

/*
 *   FUNCTION ARITHMATIC
 */

void R6502::SBC( BytePair &Value )
{
	Value.Unsigned = ~Value.Unsigned;
	ADC(Value);
}

void R6502::ADC( BytePair &Value )
{
	Working.Word = A + Value.Unsigned + (Carry ? 1 : 0);
	Overflow	= ((~(A^Value.Unsigned))&(A^Working.Byte)&0x80) != 0;			
	Carry		= (Working.Unsigned.ByteH != 0);

	A = Working.Byte;

	Negitive	= (A & 0x80) != 0;
	Zero		= (A == 0);
}

void R6502::CMP( BytePair &Value )
{
	Working.Word = A - Value.Unsigned;
	Zero		= Working.Byte == 0;
	Negitive	= (Working.Byte & 0x80) != 0;
	Carry		= (Working.Word & 0x100) == 0;
}

void R6502::CPX( BytePair &Value )
{
	Working.Word =  X - Value.Unsigned;

	Zero		= Working.Byte == 0;
	Negitive	= (Working.Byte & 0x80) != 0;
	Carry		= (Working.Word & 0x100) == 0;
}
void R6502::CPY( BytePair &Value )
{
	Working.Word =  Y - Value.Unsigned;

	Zero		= Working.Byte == 0;
	Negitive	= (Working.Byte & 0x80) != 0;
	Carry		= (Working.Word & 0x100) == 0;
}
void R6502::DEC( BytePair &Value ) 
{
	Value.Unsigned--;
	Zero		= (Value.Unsigned == 0);
	Negitive	= (Value.Unsigned & 0x80) != 0;
}
void R6502::INC( BytePair &Value )
{
	Value.Unsigned++;
	Zero		= (Value.Unsigned == 0);
	Negitive	= (Value.Unsigned & 0x80) != 0;
}
void R6502::LSR( BytePair &Value )
{
	Carry		= (Value.Unsigned & 0x01) != 0;
	Value.Unsigned >>= 1;
	Negitive	= false;
	Zero		= (Value.Unsigned == 0);
}
void R6502::ASL( BytePair &Value ) 
{
	Carry		= (Value.Unsigned & 0x80) != 0;
	Value.Unsigned <<= 1;
	Negitive	= (Value.Unsigned & 0x80) != 0;
	Zero		= (Value.Unsigned == 0);
}
void R6502::ROL( BytePair &Value )
{
	Working.Byte = (Value.Unsigned << 1) | (Carry ? 0x01: 0x00);
	Zero				= (Working.Byte == 0);
	Carry				= (Value.Unsigned & 0x80) != 0;
	Negitive			= (Working.Byte & 0x80) != 0;
	Value.Unsigned	= Working.Byte;
}
void R6502::ROR( BytePair &Value ) 
{
	Working.Byte = (Value.Unsigned >> 1) | (Carry ? 0x80: 0x00);
	Zero				= (Working.Byte == 0);
	Carry				= (Value.Unsigned & 0x01) != 0;
	Negitive			= (Working.Byte & 0x80) != 0;
	Value.Unsigned = Working.Byte;
}
void R6502::BIT( BytePair &Value ) 
{
	Overflow	= (Value.Unsigned & 0x40) != 0;
	Negitive	= (Value.Unsigned & 0x80) != 0;
	Value.Unsigned &= A;
	Zero		= (Value.Unsigned == 0);
}
void R6502::AND( BytePair &Value ) 
{
	A			&= Value.Unsigned;
	Zero		= (A == 0);
	Negitive	= (A & 0x80) != 0;
}
void R6502::EOR( BytePair &Value ) 
{
	A			^= Value.Unsigned;
	Zero		= (A == 0);
	Negitive	= (A & 0x80) != 0;
}
void R6502::ORA( BytePair &Value )
{
	A			|= Value.Unsigned;
	Zero		= (A == 0);
	Negitive	= (A & 0x80) != 0;
}
void R6502::LDA( BytePair &Value ) 
{
	A			= Value.Unsigned;
	Zero		= (A == 0);
	Negitive	= (A & 0x80) != 0;
}
void R6502::LDX( BytePair &Value ) 
{
	X			= Value.Unsigned;
	Zero		= (X == 0);
	Negitive	= (X & 0x80) != 0;
}
void R6502::LDY( BytePair &Value ) 
{
	Y			= Value.Unsigned;
	Zero		= (Y == 0);
	Negitive	= (Y & 0x80) != 0;
}

void R6502::STA( BytePair &Value )
{
	Value.Unsigned = A;
}

void R6502::STX( BytePair &Value )
{
	Value.Unsigned = X;
}

void R6502::STY( BytePair &Value )
{
	Value.Unsigned = Y;
}

void R6502::NOP( BytePair &Value )
{
}

/* GLITCH OPCODES */
void R6502::LAX( BytePair &Value )
{
	X = A		= Value.Unsigned;
	Zero		= (X == 0);
	Negitive	= (X & 0x80) != 0;
}

void R6502::SAX( BytePair &Value )
{
	Value.Unsigned = X & A;
}

void R6502::DCP( BytePair &Value )
{
	Working.Word = A - (--Value.Unsigned);
	Zero		= Working.Byte == 0;
	Negitive	= (Working.Byte & 0x80) != 0;
	Carry		= (Working.Word & 0x100) == 0;
}

void R6502::ISB( BytePair &Value )
{
	Value.Unsigned++;
	Working.Word = A - Value.Unsigned - (Carry ? 0 : 1);		
	Overflow	= ((A ^ Value.Unsigned)&(A ^ Working.Byte)&0x80) != 0;			
	Carry		= (Working.Unsigned.ByteH == 0);

	A = Working.Byte;

	Negitive	= (A & 0x80) != 0;
	Zero		= (A == 0);
}

void R6502::SLO( BytePair &Value )
{
	Carry = (Value.Unsigned & 0x80) != 0;
	Value.Unsigned <<= 1;
	A |= Value.Unsigned;
	Zero		= A == 0;
	Negitive	= (A & 0x80) != 0;
}

void R6502::RLA( BytePair &Value )
{
	Working.Byte = (Value.Unsigned << 1) | (Carry ? 0x01: 0x00);
	Carry = (Value.Unsigned & 0x80) != 0;
	Value.Unsigned = Working.Byte;
	A &= Value.Unsigned;

	Zero		= A == 0;
	Negitive	= (A & 0x80) != 0;
}

void R6502::SRE( BytePair &Value )
{
	Carry = (Value.Unsigned & 0x01) != 0;
	Value.Unsigned >>= 1;
	A ^= Value.Unsigned;
	Zero		= A == 0;
	Negitive	= (A & 0x80) != 0;
}

void R6502::RRA( BytePair &Value )
{
	Working.Byte = (Value.Unsigned >> 1) | (Carry ? 0x80 : 0);
	Carry = (Value.Unsigned & 0x01) != 0;
	Value.Unsigned = Working.Byte;

	Working.Word = A + Value.Unsigned + (Carry ? 1 : 0);
	Overflow	= ((~(A^Value.Unsigned))&(A^Working.Byte)&0x80) != 0;			
	Carry		= (Working.Unsigned.ByteH != 0);

	A = Working.Byte;

	Negitive	= (A & 0x80) != 0;
	Zero		= (A == 0);
}

void R6502::ANC( BytePair &Value )
{
	printf( "Attempted undefined opcode\n" );
}
void R6502::ASR( BytePair &Value )
{
	printf( "Attempted undefined opcode\n" );
}
void R6502::ARR( BytePair &Value )
{
	printf( "Attempted undefined opcode\n" );
}
void R6502::ANE( BytePair &Value )
{
	printf( "Attempted undefined opcode\n" );
}
void R6502::SHA( BytePair &Value )
{
	printf( "Attempted undefined opcode\n" );
}
void R6502::SHS( BytePair &Value )
{
	printf( "Attempted undefined opcode\n" );
}
void R6502::SHX( BytePair &Value )
{
	printf( "Attempted undefined opcode\n" );
}
void R6502::SHY( BytePair &Value )
{
	printf( "Attempted undefined opcode\n" );
}
void R6502::LXA( BytePair &Value )
{
	printf( "Attempted undefined opcode\n" );
}
void R6502::LAS( BytePair &Value )
{
	printf( "Attempted undefined opcode\n" );
}
void R6502::SBX( BytePair &Value )
{
	printf( "Attempted undefined opcode\n" );
}

void R6502::INX_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	X++;
	Zero		= (X == 0);
	Negitive	= (X & 0x80) != 0;
}

void R6502::INY_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	Y++;
	Zero		= (Y == 0);
	Negitive	= (Y & 0x80) != 0;
}

void R6502::DEX_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	X--;
	Zero		= (X == 0);
	Negitive	= (X & 0x80) != 0;
}

void R6502::DEY_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	Y--;
	Zero		= (Y == 0);
	Negitive	= (Y & 0x80) != 0;
}

void R6502::TAX_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	X = A;
	Zero		= (X == 0);
	Negitive	= (X & 0x80) != 0;
}

void R6502::TAY_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	Y = A;
	Zero		= (Y == 0);
	Negitive	= (Y & 0x80) != 0;
}

void R6502::TSX_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	X			= Stack.Byte;
	Zero		= (X == 0);
	Negitive	= (X & 0x80) != 0;
}

void R6502::TXA_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	A = X;
	Zero		= (A == 0);
	Negitive	= (A & 0x80) != 0;
}

void R6502::TXS_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	Stack.Byte = X;
}

void R6502::TYA_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	A = Y;
	Zero		= (A == 0);
	Negitive	= (A & 0x80) != 0;
}

void R6502::SEC_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	Carry = true;
}

void R6502::SED_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	Decimal = true;
}

void R6502::SEI_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	IRQDisable = true;
}

void R6502::CLC_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	Carry = false;
}

void R6502::CLD_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	Decimal = false;
}

void R6502::CLI_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	IRQDisable = false;
}

void R6502::CLV_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	Overflow = false;
}

// ACCUMULATOR MODES FOR SHIFTED INSTRUCTIONS
void R6502::LSR_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	Carry		= (A & 0x01) != 0;
	A >>= 1;
	Negitive	= false;
	Zero		= (A == 0);
}

void R6502::ASL_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	Carry		= (A & 0x80) != 0;
	A <<= 1;
	Negitive	= (A & 0x80) != 0;
	Zero		= (A == 0);
}

void R6502::ROL_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	Working.Byte = (A << 1) | (Carry ? 0x01: 0x00);
	Zero		= (Working.Byte == 0);
	Negitive	= (Working.Byte & 0x80) != 0;
	Carry		= (A & 0x80) != 0;
	A = Working.Byte;
}

void R6502::ROR_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();

	Working.Byte = (A >> 1) | (Carry ? 0x80: 0x00);
	Zero		= (Working.Byte == 0);
	Negitive	= (Working.Byte & 0x80) != 0;
	Carry		= (A & 0x01) != 0;
	A = Working.Byte;
}

void R6502::NOP_I()
{
	ClockHalt();
	Read(PC.Word);
	EndInstruction();
}
