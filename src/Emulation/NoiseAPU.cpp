#include "Emulation.h"

/*
 *  Notes: Does not delay resets to Sweep and Envelope counters
 *			Envelope may be off by 1
 */

NoiseAPU::NoiseAPU()
{
	// Initially silenced
	m_EnableCounter = false;
	m_LengthCounter = 0;

	m_Feedback		= 1;
	m_Period		= 0;
	m_Overflow		= 0;
	m_Volume		= 0;
	m_Enveloped		= 0;
	m_EnvPeriod		= 0;
}

void NoiseAPU::WriteEnvelope( unsigned char Byte )
{
	m_LoopEnvHalt	= (Byte & 0x20) != 0;
	m_Enveloped		= (Byte & 0x10) == 0;

	m_EnvPeriod		= (Byte & 0x0F);
}

void NoiseAPU::WriteModePeriod( unsigned char Byte )
{
	m_Mode   = Byte & 0x80;
	m_Period = NoisePeriods[Byte&0xF];
}

void NoiseAPU::WriteLength( unsigned char Byte )
{
	if( m_EnableCounter )
		m_LengthCounter = LengthTable[Byte>>3];
	m_EnvRestart = true;
}

void NoiseAPU::WriteEnable( bool enable )
{
	m_EnableCounter = enable;

	if( !m_EnableCounter )
		m_LengthCounter = 0;
}


void NoiseAPU::ClockLengthCounter()
{
	if( !m_LoopEnvHalt && m_LengthCounter > 0 )
		m_LengthCounter--;
}

void NoiseAPU::ClockEnvelope()
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

bool NoiseAPU::LengthActive()
{
	return m_LengthCounter > 0;
}

int NoiseAPU::ClockOutput()
{
	if( m_LengthCounter <= 0 )
		return 0;

	if( m_Overflow-- <= 0 )
	{
		m_Feedback = (m_Feedback >> 1) | (0x4000 &
			((m_Feedback << 14) ^ (m_Mode?(m_Feedback << 8):(m_Feedback << 13)) ));

		m_Output = m_Feedback & 1;
		m_Overflow = m_Period;
	}

	if( m_Output )
		return m_Enveloped ? m_Volume : m_EnvPeriod;
	else
		return 0;
}
