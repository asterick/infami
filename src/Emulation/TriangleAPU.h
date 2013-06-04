#ifndef __TRIANGLEAPU_H
#define __TRIANGLEAPU_H

const int TriangleWave[32] = {
	0xF,0xE,0xD,0xC,0xB,0xA,0x9,0x8,
	0x7,0x6,0x5,0x4,0x3,0x2,0x1,0x0,
	0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,
	0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF
};

class TriangleAPU
{
public:
	TriangleAPU();

	void WriteLinearCounter( unsigned char Byte );
	void WritePeriodLow( unsigned char Byte );
	void WriteLengthPeriod( unsigned char Byte );
	void WriteEnable( bool enable );

	bool LengthActive();
	
	int ClockOutput();
	void ClockLengthCounter();
	void ClockLinearCounter();
private:
	int		m_Period;
	int		m_Overflow;
	int		m_Sequence;
	int		m_Output;

	bool	m_Control;
	bool	m_HaltLinear;
	int		m_LinearCounter;
	int		m_LinearReloadValue;
	
	bool	m_EnableCounter;
	int		m_LengthCounter;
};

#endif