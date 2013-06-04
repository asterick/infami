#include "Emulation.h"

APU::APU(  const TimingSettings* timing ) :
	Square1(false), Square2(true)
{
	m_FrameTarget = timing->FrameCount;
	m_FrameError  = 30;
	m_FrameIRQ = false;
	
	m_SequenceLength = 4;
	m_InhibitIRQ = false;
	m_FrameCount = 0;	

	CtrlPort1 = NULL;
	CtrlPort2 = NULL;

	for( int i = 1; i < 31; i++ )
		SquareVolume[i] = 95.52f / (8128.0f / i + 100.0f);
	for( int i = 1; i < 203; i++ )
		OtherVolume[i]  = 163.67f / (24329.0f / i + 100.0f);

	SquareVolume[0] = OtherVolume[0] = 0;

	// Start out with pure silence
	m_OutputVolume = 0;
	m_OutputSamples = 0;
}

void APU::InsertController( bool Player2, Controller *ctrl )
{
	if( !Player2 )
		CtrlPort1 = ctrl;
	else
		CtrlPort2 = ctrl;
}

bool APU::FrameIRQ()
{
	return m_FrameIRQ || DeltaModAPU::IRQActive();
}

void APU::APUWrite( unsigned short Address, unsigned char Byte )
{
	switch( Address )
	{
		/* Square 1 */
		case 0x4000:
			Square1.WriteDutyEnv(Byte);
			break ;
		case 0x4001:
			Square1.WriteSweep(Byte);
			break ;
		case 0x4002:
			Square1.WritePeriodLow(Byte);
			break ;
		case 0x4003:
			Square1.WriteLengthPeriod(Byte);
			break ;
		/* Square 2 */
		case 0x4004:
			Square2.WriteDutyEnv(Byte);
			break ;
		case 0x4005:
			Square2.WriteSweep(Byte);
			break ;
		case 0x4006:
			Square2.WritePeriodLow(Byte);
			break ;
		case 0x4007:
			Square2.WriteLengthPeriod(Byte);
			break ;
		/* Triangle */
		case 0x4008:
			Triangle.WriteLinearCounter(Byte);
			break ;
		case 0x400A:
			Triangle.WritePeriodLow(Byte);
			break ;
		case 0x400B:
			Triangle.WriteLengthPeriod(Byte);
			break ;
		/* Noise */
		case 0x400C:
			Noise.WriteEnvelope(Byte);
			break ;
		case 0x400E:
			Noise.WriteModePeriod(Byte);
			break ;
		case 0x400F:
			Noise.WriteLength(Byte);
			break ;
		/* DCM */
		case 0x4010:
			DeltaModAPU::WriteModeFreq(Byte);
			break ;
		case 0x4011:
			DeltaModAPU::WriteDAC(Byte);
			break ;
		case 0x4012:
			DeltaModAPU::WriteAddress(Byte);
			break ;
		case 0x4013:
			DeltaModAPU::WriteLength(Byte);
			break ;
		/* CPU Control / IO */
		case 0x4014:	// Sprite DMA
			FireSpriteDMA( Byte );
			break ;
		case 0x4015:	// Sound / Vertical Clock Signal Register
			Square1.WriteEnable( (Byte & 0x01) != 0 );
			Square2.WriteEnable( (Byte & 0x02) != 0 );
			Triangle.WriteEnable( (Byte & 0x04) != 0 );
			Noise.WriteEnable( (Byte & 0x08) != 0 );
			DeltaModAPU::WriteEnable( (Byte & 0x10) != 0 );

			break ;
		case 0x4016:	// Joypad 1
			if( CtrlPort1 )
				CtrlPort1->Strobe( Byte );
			if( CtrlPort2 )
				CtrlPort2->Strobe( Byte );
			break ;

		case 0x4017:	// Joypad 2 / Frame Count Control
			m_InhibitIRQ = (Byte & 0x40) ? true : false;
			m_FrameError = 0;

			if( m_InhibitIRQ )
				m_FrameIRQ = false;						

			if( Byte & 0x80 )
			{
				m_SequenceLength = 5;
				m_FrameCount = 1;
				Square1.ClockSweep();
				Square2.ClockSweep();
				Triangle.ClockLengthCounter();
				Noise.ClockLengthCounter();
			}
			else
			{
				m_SequenceLength = 4;
				m_FrameCount = 0;
			}

			break ;
	}
}

unsigned char APU::APURead( unsigned short Address, unsigned char Trash )
{
	switch( Address )
	{
		/* CPU Control / IO */
		case 0x4015:	// Sound / Vertical Clock Signal Register
			Trash &= 0x20;	// Only one byte of trash

			Trash |= DeltaModAPU::IRQActive() ? 0x80 : 0;
			Trash |= m_FrameIRQ ? 0x40 : 0;
			Trash |= DeltaModAPU::LengthActive() ? 0x10 : 0;
			Trash |= Noise.LengthActive() ? 0x08 : 0;
			Trash |= Triangle.LengthActive() ? 0x04 : 0;
			Trash |= Square2.LengthActive() ? 0x02 : 0;
			Trash |= Square1.LengthActive() ? 0x01 : 0;

			m_FrameIRQ = false;
			return Trash ;
		case 0x4016:	// Joypad 1
			if( CtrlPort1 )
				return CtrlPort1->ReadPort( Trash );
			return Trash;
		case 0x4017:	// Joypad 2
			if( CtrlPort2 )
				return CtrlPort2->ReadPort( Trash );
			return Trash;
	}
	return Trash;
}

void APU::FrameClock( int cycles )
{
	if( m_FrameError >= m_FrameTarget )
	{
		m_FrameError -= m_FrameTarget;
		
		if( m_SequenceLength == 5 )
		{
			switch( ++m_FrameCount )
			{
			case 1:
			case 3:
				Square1.ClockSweep();
				Square2.ClockSweep();
				Triangle.ClockLengthCounter();
				Noise.ClockLengthCounter();
			case 2:
			case 4:
				Square1.ClockEnvelope();
				Square2.ClockEnvelope();
				Triangle.ClockLinearCounter();
				Noise.ClockEnvelope();
				break ;
			case 5:
				m_FrameCount = 0;
				break ;
			}
		}
		else
		{
			switch( ++m_FrameCount )
			{
			case 4:
				if( !m_InhibitIRQ )
					m_FrameIRQ = true;
				m_FrameCount = 0;
			case 2:
				Square1.ClockSweep();
				Square2.ClockSweep();
				Triangle.ClockLengthCounter();
				Noise.ClockLengthCounter();
			case 1:
			case 3:
				Square1.ClockEnvelope();
				Square2.ClockEnvelope();
				Triangle.ClockLinearCounter();
				Noise.ClockEnvelope();
				break ;
			}
		}
	}

	m_FrameError += cycles;
}

void APU::AudioClock( bool Muted )
{
	if( !Muted )
	{
		m_OutputVolume += SquareVolume[Square1.ClockOutput() + Square2.ClockOutput()];
		m_OutputVolume += OtherVolume[ Triangle.ClockOutput()*3 + Noise.ClockOutput()*2 + DeltaModAPU::ClockOutput() ];
		m_OutputSamples++;
	}
	else
	{
		DeltaModAPU::ClockOutput();
	}
}

float APU::StreamAudio()
{
	float output = m_OutputVolume / m_OutputSamples;
	m_OutputVolume = 0;
	m_OutputSamples = 0;

	return output;
}
