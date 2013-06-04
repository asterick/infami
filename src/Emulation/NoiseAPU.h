#ifndef __NOISEAPU_H
#define __NOISEAPU_H

const int NoisePeriods[16] =
{
	0x003, 0x007, 0x00F, 0x01F,
	0x03F, 0x05F, 0x07F, 0x09F,
	0x0C9, 0x0FD, 0x17B, 0x1FB,
	0x2F9, 0x3F7, 0x7F1, 0xFE3
};

class NoiseAPU
{
public:
	NoiseAPU();

	void WriteEnvelope( unsigned char Byte );
	void WriteModePeriod( unsigned char Byte );
	void WriteLength( unsigned char Byte );
	void WriteEnable( bool enable );

	bool LengthActive();
	
	int ClockOutput();
	void ClockLengthCounter();
	void ClockEnvelope();
private:
	int		m_Feedback;
	int		m_Period;
	int		m_Overflow;
	int		m_Output;
	int		m_Mode;

	bool	m_LoopEnvHalt;
	
	bool	m_EnableCounter;
	int		m_LengthCounter;

	int		m_Volume;
	bool	m_Enveloped;
	bool	m_EnvRestart;
	int		m_EnvPeriod;
	int		m_EnvCounter;
};

#endif