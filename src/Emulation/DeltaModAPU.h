#ifndef __DELTAMODAPU_H
#define __DELTAMODAPU_H

const int DeltaPeriods[16] = 
{
	0x1AC, 0x17C, 0x154, 0x140,
	0x11E, 0x0FE, 0x0E2, 0x0D6,
	0x0BE, 0x0A0, 0x08E, 0x080,
	0x06A, 0x054, 0x048, 0x036
};

class DeltaModAPU
{
public:
	DeltaModAPU();
	bool RequestHalt();
	bool IRQActive();
	bool LengthActive();

	void WriteModeFreq( unsigned char Byte );
	void WriteDAC( unsigned char Byte );
	void WriteAddress( unsigned char Byte );
	void WriteLength( unsigned char Byte );
	void WriteEnable( bool );
	int ClockOutput();

	virtual unsigned char DMCBusRead( unsigned short addr ) { return 0; }
private:
	bool m_RequestIRQ;
	bool m_Halt;

	bool m_Silence;
	bool m_Empty;

	int m_Period;
	int m_Counter;
	bool m_GenerateIRQ;
	bool m_Loop;

	unsigned short m_ResetAddress;
	unsigned short m_LoadAddress;
	int m_ResetLength;
	int m_BytesLeft;

	int m_ReaderCounter;
	int m_ShifterCounter;
	int m_ShifterSequence;

	unsigned char ReadBuffer;
	unsigned char ShiftBuffer;

	int Output;
};

#endif