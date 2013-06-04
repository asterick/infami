#ifndef __6502_H
#define __6502_H

#define APU_SPR_DMA_TARGET	0x2004

union BytePair
{
	unsigned char	Unsigned;
	signed char		Signed;
};

class R6502
{
public:
	R6502();
	
	void Restart();
protected:
	void CPUStep();
	void SpriteDMA( unsigned char Base );

	// Expected overrides
	virtual unsigned char Read( unsigned short addr ) { return 0; }
	virtual void Write( unsigned short addr, unsigned char ch ) {}
	virtual bool NMIActive() { return false; }
	virtual bool IRQActive() { return false; }
	virtual bool HaltActive() { return false; }
	virtual void Clock() {}
private:
	void ClockHalt();

	unsigned char CompileP( bool Break );
	void DecompileP( unsigned char p );
	void EndInstruction();

	void Reset();
	void NonMaskInterrupt();
	void MaskableInterrupt();
	void SpriteDMA();


	// Execution state aids
	unsigned char	m_OpCode;
	int				m_EndInstructionResult;	

	/* Registers */
	unsigned char A;
	unsigned char X;
	unsigned char Y;

	union WordValue
	{
		unsigned short	Word;
		unsigned char	Byte;
		signed char		SByte;

		#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			struct
			{
				unsigned char ByteL;
				unsigned char ByteH;
			} Unsigned;

			struct
			{
				signed char ByteL;
				signed char ByteH;
			} Signed;
		#else
			struct
			{
				unsigned char ByteH;
				unsigned char ByteL;
			} Unsigned;

			struct
			{
				signed char ByteH;
				signed char ByteL;
			} Signed;
		#endif
	};

	BytePair Arguement;

	WordValue PC;
	WordValue Effective;
	WordValue Working;
	WordValue Stack;

	// Process Flags
	bool Decimal;
	bool IRQDisable;

	// Math flags
	bool Negitive;
	bool Overflow;
	bool Zero;
	bool Carry;	

	// SPR-DMA Execution values
	union
	{
		unsigned short Address;
		#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			struct
			{
				unsigned char Bytes;
				unsigned char Base;
			} Units;
		#else
			struct
			{
				unsigned char Base;
				unsigned char Bytes;
			} Units;
		#endif
	} SprDma;
	
	bool			m_SpriteDMA;
	bool			m_NMIAck;		// This is an edge sensitive value, so the CPU keeps it logged


	// --- INSTRUCTION HELPERS ---
	void Branch();
public:
	void SharedRead();
	void SharedReadWrite();
	void SharedWrite();

	void Break();
	void Branch_NC();
	void Branch_NS();
	void Branch_VC();
	void Branch_VS();
	void Branch_CC();
	void Branch_CS();
	void Branch_ZC();
	void Branch_ZS();
	void JumpSub();
	void Jump();
	void JumpIndirect();
	void ReturnInterrupt();
	void ReturnSub();
	void PushA();
	void PushP();
	void PopA();
	void PopP();
	void Jam();

	void Immediate();
	void Absolute();
	void Zeropage();
	void AbsoluteX_R();
	void AbsoluteX_W();
	void AbsoluteY_R();
	void AbsoluteY_W();
	void ZeropageX();
	void ZeropageY();

	void IndexedIndirect();

	void IndirectIndexed_R();
	void IndirectIndexed_W();

	void SBC( BytePair &Immediate );
	void ADC( BytePair &Immediate );
	void CMP( BytePair &Immediate );
	void CPX( BytePair &Immediate );
	void CPY( BytePair &Immediate );
	void DEC( BytePair &Immediate ); 
	void INC( BytePair &Immediate );
	void LSR( BytePair &Immediate );
	void ASL( BytePair &Immediate ); 
	void ROL( BytePair &Immediate );
	void ROR( BytePair &Immediate ); 
	void BIT( BytePair &Immediate ); 
	void AND( BytePair &Immediate ); 
	void EOR( BytePair &Immediate ); 
	void ORA( BytePair &Immediate );
	void LDA( BytePair &Immediate ); 
	void LDX( BytePair &Immediate ); 
	void LDY( BytePair &Immediate ); 
	void STA( BytePair &Immediate );
	void STX( BytePair &Immediate );
	void STY( BytePair &Immediate );
	void NOP( BytePair &Immediate );
	void LAX( BytePair &Immediate );
	void SAX( BytePair &Immediate );
	void DCP( BytePair &Immediate );
	void ISB( BytePair &Immediate );
	void SLO( BytePair &Immediate );
	void RLA( BytePair &Immediate );
	void SRE( BytePair &Immediate );
	void RRA( BytePair &Immediate );
	void ANC( BytePair &Immediate );
	void ASR( BytePair &Immediate );
	void ARR( BytePair &Immediate );
	void ANE( BytePair &Immediate );
	void SHA( BytePair &Immediate );
	void SHS( BytePair &Immediate );
	void SHX( BytePair &Immediate );
	void SHY( BytePair &Immediate );
	void LXA( BytePair &Immediate );
	void LAS( BytePair &Immediate );
	void SBX( BytePair &Immediate );

	void INX_I();
	void INY_I();
	void DEX_I();
	void DEY_I();
	void TAX_I();
	void TAY_I();
	void TSX_I();
	void TXA_I();
	void TXS_I();
	void TYA_I();
	void SEC_I();
	void SED_I();
	void SEI_I();
	void CLC_I();
	void CLD_I();
	void CLI_I();
	void CLV_I();
	void LSR_I();
	void ASL_I();
	void ROL_I();
	void ROR_I();
	void NOP_I();
};



#endif