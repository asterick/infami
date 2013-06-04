#ifndef __SQUAREAPU_H
#define __SQUAREAPU_H

class SquareAPU
{
public:
	SquareAPU( bool channel2 );

	void WriteDutyEnv( unsigned char Byte );
	void WriteSweep( unsigned char Byte );
	void WritePeriodLow( unsigned char Byte );
	void WriteLengthPeriod( unsigned char Byte );
	void WriteEnable( bool enable );

	bool LengthActive();
	
	int ClockOutput();
	void ClockSweep();
	void ClockEnvelope();
private:
	bool	m_Channel2;
	int		m_Duty;
	int		m_Period;
	int		m_Overflow;
	int		m_Sequence;
	int		m_Output;

	bool	m_Sweeped;
	bool	m_SweepWrite;
	bool	m_SweepNegitive;
	int		m_SweepPeriod;
	int		m_SweepCounter;
	int		m_SweepShift;
	bool	m_SweepOn;

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