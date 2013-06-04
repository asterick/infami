#include <memory.h>

#include "../Emulation/Emulation.h"
#include "VRC6.h"

VRC6::VRC6( const TimingSettings *Timing, int bit0, int bit1 ) : 
	ROM(Timing) 
{
	m_Bit0 = bit0;
	m_Bit1 = bit1;
}

void VRC6::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	PrgMaskLo	= (PRGSize - 1) & ~0x3FFF;
	PrgMaskHi	= (PRGSize - 1) & ~0x1FFF;
	
	m_CHRMask	= (CHRSize - 1) & ~0x03FF;

	Mirroring	= 0;	// Vertical

	m_PRGPage0	= 0;
	m_PRGPage1	= 0;

	m_CHRPage0	= 0;
	m_CHRPage1	= 0;
	m_CHRPage2	= 0;
	m_CHRPage3	= 0;
	m_CHRPage4	= 0;
	m_CHRPage5	= 0;
	m_CHRPage6	= 0;
	m_CHRPage7	= 0;

	m_CounterReload = 0;
	m_Counter = 0;

	m_IRQActive = false;
	m_IRQEnaAck = false;
	m_IRQEnable = false;
	m_IRQMode	= false;

	m_AudioAccumulator = 0;
	m_AudioSamples = 0;
	m_Ch1Enable = false;
	m_Ch2Enable = false;
	m_Ch3Enable = false;
	m_Ch1Freq = 0xFFF;
	m_Ch2Freq = 0xFFF;
	m_Ch3Freq = 0xFFF;
	m_Ch1Overflow = 0;
	m_Ch2Overflow = 0;
	m_Ch3Overflow = 0;
	m_Ch1Sequencer = 0;
	m_Ch2Sequencer = 0;
	m_Ch3Sequencer = 0;
	m_Ch1Volume = 0;
	m_Ch2Volume = 0;
	m_Ch1Duty = 0xFF00;
	m_Ch2Duty = 0xFF00;

	m_Ch3Accumulator = 0;
	m_Ch3AccumRate = 0;

	m_Ch1Output = 0;
	m_Ch2Output = 0;
	m_Ch3Output = 0;

	m_Scalar = 341;
}

bool VRC6::IRQActive()
{
	return m_IRQActive;
}

float VRC6::StreamAudio()
{
	float output = ROM::StreamAudio();

	output += m_AudioAccumulator / 128.0f / m_AudioSamples;

	m_AudioSamples = 0;
	m_AudioAccumulator = 0;

	return output;
}

void VRC6::CPUClock()
{
	if( m_Ch1Enable )
	{
		if( m_Ch1Overflow++ >= m_Ch1Freq )
		{
			m_Ch1Output		= ((m_Ch1Duty >> m_Ch1Sequencer) & 1) ? m_Ch1Volume : 0;

			m_Ch1Overflow	= 0;
			m_Ch1Sequencer	= (m_Ch1Sequencer - 1) & 0xF;
		}

		m_AudioAccumulator += m_Ch1Output;
	}

	if( m_Ch2Enable )
	{
		if( m_Ch2Overflow++ >= m_Ch2Freq )
		{
			m_Ch2Output		= ((m_Ch2Duty >> m_Ch2Sequencer) & 1) ? m_Ch2Volume : 0;

			m_Ch2Overflow	= 0;
			m_Ch2Sequencer	= (m_Ch2Sequencer - 1) & 0xF;
		}

		m_AudioAccumulator += m_Ch2Output;
	}

	if( m_Ch3Enable )
	{
		if( m_Ch3Overflow++ >= m_Ch3Freq )
		{			
			if( ++m_Ch3Sequencer == 14 )
			{
				m_Ch3Sequencer = 0;
				m_Ch3Accumulator = 0;
			}
			else if( m_Ch3Sequencer & 1 )
			{
				m_Ch3Accumulator += m_Ch3AccumRate;
			}
			
			m_Ch3Output = m_Ch3Accumulator >> 3;
			m_Ch3Overflow	= 0;
		}

		m_AudioAccumulator += m_Ch3Output;
	}

	m_AudioSamples++;

	if( !m_IRQEnable )
		return ;
	
	if( !m_IRQMode )
	{
		m_Scalar -= 3;

		if( m_Scalar <= 0 )
			m_Scalar += 341;
		else
			return;
	}

	if( m_Counter++ == 0xFF )
	{
		m_IRQActive = true;
		m_Counter = m_CounterReload;
	}
}

void VRC6::WriteReg( unsigned short addr, unsigned char ch )
{
	// Do some magic to convert VRC2 / VRC4 registers uniformly
	unsigned short reg = (addr & 0xF000) |
		(( addr & m_Bit0 ) ? 1 : 0) |
		(( addr & m_Bit1 ) ? 2 : 0);

	switch( reg )
	{
	case 0x8000: case 0x8001: case 0x8002: case 0x8003:
		m_PRGPage0 = (ch << 14) & PrgMaskLo;
		break ;
	case 0xC000: case 0xC001: case 0xC002: case 0xC003:
		m_PRGPage1 = (ch << 13) & PrgMaskHi;
		break ;
	case 0xB003:
		switch( (ch >> 2) & 0x3 )
		{
		case 0:
			SetMirrorVertical();
			break ;
		case 1:
			SetMirrorHorizontal();
			break ;
		case 2:
			SetMirrorZero();
			break ;
		case 3:
			SetMirrorOne();
			break ;
		}

		break ;

	case 0xD000:
		m_CHRPage0 = (ch << 10) & m_CHRMask;
		break ;
	case 0xD001:
		m_CHRPage1 = (ch << 10) & m_CHRMask;
		break ;
	case 0xD002:
		m_CHRPage2 = (ch << 10) & m_CHRMask;
		break ;
	case 0xD003:
		m_CHRPage3 = (ch << 10) & m_CHRMask;
		break ;
	case 0xE000:
		m_CHRPage4 = (ch << 10) & m_CHRMask;
		break ;
	case 0xE001:
		m_CHRPage5 = (ch << 10) & m_CHRMask;
		break ;
	case 0xE002:
		m_CHRPage6 = (ch << 10) & m_CHRMask;
		break ;
	case 0xE003:
		m_CHRPage7 = (ch << 10) & m_CHRMask;
		break ;

	case 0xF000:
		m_CounterReload = ch;
		break ;
	case 0xF001:
		m_IRQEnaAck = (m_BusTrash & 0x1) != 0;
		m_IRQEnable = (m_BusTrash & 0x2) != 0;
		m_IRQMode = (m_BusTrash & 0x4) != 0;
		if( m_IRQEnable )
		{
			m_Counter = m_CounterReload;
			m_Scalar = 341;
		}
		m_IRQActive = false;
		break ;
	case 0xF002:
		m_IRQActive = false;
		m_IRQEnable = m_IRQEnaAck;
		break ;

	// VRC6 Audio:
	case 0x9000:
		if( ch & 0x80 )
			m_Ch1Duty = 0xFFFF;
		else
			m_Ch1Duty = (unsigned short)(0xFFFF8000 >> ((ch & 0x70) >> 4));
		m_Ch1Volume = ch & 0xF;
		break ;
	case 0xA000: 
		if( ch & 0x80 )
			m_Ch2Duty = 0xFFFF;
		else
			m_Ch2Duty = (unsigned short)(0xFFFF8000 >> ((ch & 0x70) >> 4));
		m_Ch2Volume = ch & 0xF;
		break ;
	case 0xB000:
		m_Ch3AccumRate = ch & 0x3F;
		break ;
	case 0x9001:
		m_Ch1Freq	= (m_Ch1Freq & 0xF00) | ch;
		break ;
	case 0xA001: 
		m_Ch2Freq	= (m_Ch1Freq & 0xF00) | ch;
		break ;
	case 0xB001: 
		m_Ch3Freq	= (m_Ch1Freq & 0xF00) | ch;
		break ;
	case 0x9002: 
		m_Ch1Enable = (ch & 0x80) != 0;
		m_Ch1Freq   = ((ch & 0x0F) << 8) | (m_Ch1Freq & 0x00FF);
		break ;
	case 0xA002: 
		m_Ch2Enable = (ch & 0x80) != 0;
		m_Ch2Freq   = ((ch & 0x0F) << 8) | (m_Ch2Freq & 0x00FF);
		break ;
	case 0xB002:
		m_Ch3Enable = (ch & 0x80) != 0;
		m_Ch3Freq   = ((ch & 0x0F) << 8) | (m_Ch3Freq & 0x00FF);
		break ;
	}
}

unsigned char VRC6::Read( unsigned short addr )
{
	switch( addr & 0xE000 )
	{
	case 0x0000:
		return m_BusTrash = SystemRam[addr&0x7FF];
	case 0x2000:
		return m_BusTrash = PPURead( addr, m_BusTrash );
	case 0x4000:
		return m_BusTrash = APURead( addr, m_BusTrash );
	case 0x6000:
		if( BCKRam )
			return m_BusTrash = BCKRam[ addr & RamMask ];
		break ;
	case 0x8000:
	case 0xA000:
		return m_BusTrash = PRGRom[(addr & 0x3FFF) | m_PRGPage0 ];
	case 0xC000:
		return m_BusTrash = PRGRom[(addr & 0x1FFF) | m_PRGPage1 ];
	case 0xE000:
		return m_BusTrash = PRGRom[addr | PrgMaskHi];
	}

	return m_BusTrash;
}

void VRC6::Write( unsigned short addr, unsigned char ch )
{
	m_BusTrash = ch;
	switch( addr & 0xE000 )
	{
	case 0x0000:
		SystemRam[addr&0x7FF] = ch;
		break ;
	case 0x2000:
		PPUWrite( addr, m_BusTrash );
		break ;
	case 0x4000:
		APUWrite( addr, m_BusTrash );
		break ;
	case 0x6000:
		if( BCKRam )
			BCKRam[ addr & RamMask ] = ch;
		break ;
	default:
		WriteReg( addr, ch );
	}
}

void VRC6::VideoAddressLatch( unsigned short Address ) 
{
	m_VideoAddress = Address; 
}

void VRC6::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		CHRRam[m_VideoAddress&0x1FFF] = Byte;
		break ;
	default:		// Name tables
		SetNameTable( m_VideoAddress, Byte );
		break ;
	}
}

unsigned char VRC6::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
		switch( m_VideoAddress & 0xC00 )
		{
		case 0x000:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage0 ];
		case 0x400:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage1 ];
		case 0x800:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage2 ];
		case 0xC00:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage3 ];
			break ;
		}
		break ;
	case 0x1000:
		switch( m_VideoAddress & 0xC00 )
		{
		case 0x000:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage4 ];
		case 0x400:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage5 ];
		case 0x800:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage6 ];
		case 0xC00:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage7 ];
			break ;
		}
		break ;
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
