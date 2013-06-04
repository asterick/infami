#include "Emulation.h"

/*
 *  Notes: Does not delay resets to Sweep and Envelope counters
 *			Envelope may be off by 1
 */

SquareAPU::SquareAPU( bool channel2 )
{
	m_Channel2	= channel2;

	// Initially silenced
	m_EnableCounter = false;
	m_LengthCounter = 0;

	m_Duty			= 0;
	m_Period		= 0;
	m_Overflow		= 0;
	m_Sequence		= 0;
	m_Volume		= 0;
	m_Enveloped		= 0;
	m_EnvPeriod		= 0;
}

void SquareAPU::WriteDutyEnv( unsigned char Byte )
{
	m_LoopEnvHalt	= (Byte & 0x20) != 0;
	m_Enveloped		= (Byte & 0x10) == 0;

	switch( Byte >> 6 )
	{
	case 0:
		m_Duty = 0x02;	// 12.5%
		break ;
	case 1:
		m_Duty = 0x06;	// 25%
		break ;
	case 2:
		m_Duty = 0x1E;	// 50%
		break ;
	case 3:
		m_Duty = 0xF9;	// 75%
		break ;
	}

	m_EnvPeriod		= Byte & 0x0F;
}

void SquareAPU::WriteSweep( unsigned char Byte )
{	
	m_Sweeped = (Byte & 0x80) != 0;
	m_SweepNegitive = (Byte & 0x08) != 0;
	m_SweepShift  = (Byte & 7);
	m_SweepPeriod = (Byte >> 4) & 7;

	if( m_Sweeped )
		m_SweepWrite = true;

	m_SweepWrite = true;
}

void SquareAPU::WritePeriodLow( unsigned char Byte )
{
	m_Period = (m_Period & 0xE00) | (Byte << 1);
}

void SquareAPU::WriteLengthPeriod( unsigned char Byte )
{
	if( m_EnableCounter )
		m_LengthCounter = LengthTable[Byte>>3];

	m_Overflow = m_Period = (m_Period & 0x1FE) | ((Byte & 0x7)<<9);
	m_Sequence = 0;
	m_EnvRestart = true;
}

void SquareAPU::WriteEnable( bool enable )
{
	m_EnableCounter = enable;

	if( !m_EnableCounter )
		m_LengthCounter = 0;
}


void SquareAPU::ClockSweep()
{
	if( !m_LoopEnvHalt && m_LengthCounter > 0 )
		m_LengthCounter--;

	if( m_Sweeped )
	{
		if( m_SweepCounter-- <= 0 )
		{
			m_SweepCounter = m_SweepPeriod;

			if( m_Period <= 0x7FF && m_Period >= 8 )
			{
				if( m_SweepNegitive )
					m_Period += (m_Channel2 ? 0 : 1) + ~(m_Period>>m_SweepShift);
				else
					m_Period += m_Period>>m_SweepShift;
			}
		}
	}

	if( m_SweepWrite )
	{
		m_SweepCounter = m_SweepPeriod;		// This may need to be changed to delay one clock
		m_SweepWrite = false;
	}
}

void SquareAPU::ClockEnvelope()
{
	if( m_Enveloped )
	{
		if( m_EnvRestart )
		{
			m_Volume		= 15;
			m_EnvCounter	= m_EnvPeriod;
			m_EnvRestart	= false;
		}
		else if( m_EnvCounter-- <= 0 )
		{
			if( m_Volume-- <= 0 )
				m_Volume = m_LoopEnvHalt ? 15 : 0;

			m_EnvCounter = m_EnvPeriod;
		}
	}
}

bool SquareAPU::LengthActive()
{
	return m_LengthCounter > 0;
}

int SquareAPU::ClockOutput()
{
	if( m_LengthCounter <= 0 || m_Period > 0x7FF || m_Period < 8 )
		return 0;

	if( m_Overflow-- <= 0 )
	{
		m_Output = (m_Duty >> m_Sequence) & 1;
		m_Sequence = (m_Sequence+1) & 7;
		m_Overflow = m_Period;
	}

	if( m_Output )
		return m_Enveloped ? m_Volume : m_EnvPeriod;
	else
		return 0;
}
