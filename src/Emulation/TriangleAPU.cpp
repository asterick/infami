#include "Emulation.h"

/*
 *  Notes: Does not delay resets to Sweep and Envelope counters
 *			Envelope may be off by 1
 */

TriangleAPU::TriangleAPU()
{
	// Initially silenced
	m_EnableCounter = false;
	
	m_LengthCounter = 0;
	m_LinearCounter = 0;
	m_Sequence = 0;
	m_Output = 0;
	m_Period = 0;
}

void TriangleAPU::WriteLinearCounter( unsigned char Byte )
{
	m_Control = (Byte & 0x80) != 0;
	m_LinearReloadValue = Byte & 0x7F;
}

void TriangleAPU::WritePeriodLow( unsigned char Byte )
{
	m_Period = (m_Period & 0x700) | Byte;
}

void TriangleAPU::WriteLengthPeriod( unsigned char Byte )
{
	if( m_EnableCounter )
		m_LengthCounter = LengthTable[Byte>>3];

	m_Overflow = m_Period = (m_Period & 0xFF) | ((Byte & 0x7)<<8);
	m_HaltLinear = true;
}

void TriangleAPU::WriteEnable( bool enable )
{
	m_EnableCounter = enable;

	if( !m_EnableCounter )
		m_LengthCounter = 0;
}


void TriangleAPU::ClockLengthCounter()
{
	if( !m_Control && m_LengthCounter > 0 )
		m_LengthCounter--;
}

void TriangleAPU::ClockLinearCounter()
{
	if( m_HaltLinear )
		m_LinearCounter = m_LinearReloadValue;	
	else if( m_LinearCounter > 0 )
		m_LinearCounter--;

	if( !m_Control )
		m_HaltLinear = false;	
}

bool TriangleAPU::LengthActive()
{
	return m_LengthCounter > 0;
}

int TriangleAPU::ClockOutput()
{
	// Frequency is bleedingly high
	if( m_Period == 0 )
		return 7;

	if( m_LengthCounter > 0 && m_LinearCounter > 0 && m_Overflow-- <= 0 )
	{
		m_Output = TriangleWave[m_Sequence];
		m_Sequence = (m_Sequence+1) & 31;
		m_Overflow = m_Period;
	}

	return m_Output;
}
