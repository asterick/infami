#include "DeltaModAPU.h"

DeltaModAPU::DeltaModAPU()
{
	m_Halt = false;
	m_RequestIRQ = false;

	m_Silence = true;
	m_Empty   = true;
	
	m_BytesLeft = 0;
	m_LoadAddress = 0;
	m_ReaderCounter = 0;
	m_ShifterCounter = 0;
	m_ShifterSequence = 0;
	m_Period = 0;

	WriteAddress(0);
	WriteLength(0);
	WriteModeFreq(0);
	WriteDAC(0);
}

bool DeltaModAPU::RequestHalt()
{
	return m_Halt;
}

bool DeltaModAPU::IRQActive()
{
	return m_RequestIRQ && m_GenerateIRQ;
}

bool DeltaModAPU::LengthActive()
{
	return m_RequestIRQ;//m_BytesLeft > 0;
}

void DeltaModAPU::WriteModeFreq( unsigned char Byte )
{
	m_GenerateIRQ = (Byte & 0x80) != 0;
	m_Loop = (Byte & 0x40) != 0;
	m_Period = DeltaPeriods[Byte & 0x0F];

	if( !m_GenerateIRQ )
		m_RequestIRQ = false;
}

void DeltaModAPU::WriteDAC( unsigned char Byte )
{
	if( m_Silence )
		Output = Byte & 0x7F;
}

void DeltaModAPU::WriteAddress( unsigned char Byte )
{
	m_ResetAddress = (Byte << 6) | 0xC000;
}

void DeltaModAPU::WriteLength( unsigned char Byte )
{
	m_ResetLength  = Byte << 4;
}

void DeltaModAPU::WriteEnable( bool Enable )
{
	if( !Enable )
	{
		m_BytesLeft = 0;
	}
	else if( m_BytesLeft <= 0 )
	{
		m_LoadAddress = m_ResetAddress;
		m_BytesLeft = m_ResetLength;
	}

	m_RequestIRQ = false;
}

int DeltaModAPU::ClockOutput()
{
	switch( m_ReaderCounter )
	{
	case 0:
		if( m_Empty && m_BytesLeft > 0 )
			m_Halt = true;
	case 1:
	case 2:
		m_ReaderCounter++;
		break ;
	default:
		if( m_Halt )
		{
			ReadBuffer = DMCBusRead( m_LoadAddress++ | 0x8000 );

			if( --m_BytesLeft <= 0 )
			{
				if( m_Loop )
				{
					m_LoadAddress = m_ResetAddress;
					m_BytesLeft = m_ResetLength;
				}
				else
				{
					m_RequestIRQ = true;
					m_BytesLeft = 0;
				}
			}

			m_Empty = false;
			m_Halt = false;
		}

		m_ReaderCounter = 0;
		break ;
	}

	if( m_ShifterCounter-- <= 0 )
	{
		if( !m_Silence )
		{
			if( (ShiftBuffer >> m_ShifterSequence) & 1 )
			{
				if( Output < 126 )
					Output += 2;
			}
			else
			{
				if( Output > 1 )
					Output -= 2;
			}
		}

		if( ++m_ShifterSequence == 8 )
		{
			m_Silence = m_Empty;
			ShiftBuffer = ReadBuffer;
			m_Empty = true;

			m_ShifterSequence = 0;
		}

		m_ShifterCounter = m_Period;
	}

	return Output;
}