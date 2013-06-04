#include <memory.h>

#include "../Emulation/Emulation.h"
#include "Sunsoft5.h"

void Sunsoft5::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	m_PRGMask = PrgMask & ~0x1FFF;
	m_CHRMask = (CHRSize - 1) & ~0x03FF;

	m_BankWritable = false;

	m_BankRegister = 0;

	m_PRGBank0 = PRGRom;
	m_PRGBank1 = PRGRom;
	m_PRGBank2 = PRGRom;
	m_PRGBank3 = PRGRom;
	
	m_CHRBank0 = CHRRom;
	m_CHRBank1 = CHRRom;
	m_CHRBank2 = CHRRom;
	m_CHRBank3 = CHRRom;
	m_CHRBank4 = CHRRom;
	m_CHRBank5 = CHRRom;
	m_CHRBank6 = CHRRom;
	m_CHRBank7 = CHRRom;
	
	m_IRQActive = false;
	m_IRQEnable = false;
	m_WriteLo	= false;

	m_Counter = 0;

	m_AudioAccumulator = 0;
	m_AudioSamples = 0;
	m_MasterDivider = 0;

	m_CH1Period = 0xFFF;
	m_CH2Period = 0xFFF;
	m_CH3Period = 0xFFF;

	m_CH1Overflow = 0;
	m_CH2Overflow = 0;
	m_CH3Overflow = 0;

	m_CH1Volume		= 0;
	m_CH2Volume		= 0;
	m_CH3Volume		= 0;

	m_CH1Output		= 0;
	m_CH2Output		= 0;
	m_CH3Output		= 0;
}

bool Sunsoft5::IRQActive()
{
	return m_IRQActive;
}

void Sunsoft5::CPUClock()
{
	if( ++m_MasterDivider == 16 )
	{
		if( m_CH1Enable )
		{
			if( m_CH1Overflow++ >= m_CH1Period )
			{
				m_CH1Output = (m_CH1Output ? 0 : m_CH1Volume);
				m_CH1Overflow = 0;
			}
			m_AudioAccumulator += m_CH1Output;
		}

		if( m_CH2Enable )
		{
			if( m_CH2Overflow++ >= m_CH2Period )
			{
				m_CH2Output = (m_CH2Output ? 0 : m_CH2Volume);
				m_CH2Overflow = 0;
			}
			m_AudioAccumulator += m_CH2Output;
		}

		if( m_CH3Enable )
		{
			if( m_CH3Overflow++ >= m_CH3Period )
			{
				m_CH3Output = (m_CH3Output ? 0 : m_CH3Volume);
				m_CH3Overflow = 0;
			}
			m_AudioAccumulator += m_CH3Output;
		}

		m_AudioSamples++;

		m_MasterDivider = 0;
	}

	if( m_Counter == 0 )
		m_IRQActive = m_IRQEnable;	
	
	m_Counter--;
}

float Sunsoft5::StreamAudio()
{
	float output = ROM::StreamAudio();

	output += m_AudioAccumulator / m_AudioSamples / 128.0f;

	m_AudioSamples = 0;
	m_AudioAccumulator = 0;

	return output;
}

unsigned char Sunsoft5::Read( unsigned short addr )
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
		if( m_PRGBank0 )
			return m_BusTrash = m_PRGBank0[ addr & 0x1FFF ];
		break ;
	case 0x8000:
		return m_BusTrash = m_PRGBank1[(addr & 0x1FFF)];
	case 0xA000:
		return m_BusTrash = m_PRGBank2[(addr & 0x1FFF)];
	case 0xC000:
		return m_BusTrash = m_PRGBank3[(addr & 0x1FFF)];
	case 0xE000:
		return m_BusTrash = PRGRom[addr | m_PRGMask];
	}

	return m_BusTrash;
}

void Sunsoft5::Write( unsigned short addr, unsigned char ch )
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
		if( m_BankWritable )
			m_PRGBank0[ addr & 0x1FFF ] = ch;
		break ;
	case 0x8000:
		m_BankRegister = ch & 0xF;
		break ;
	case 0xC000:
		m_AudioAddress = ch & 0xF;
		break ;
	case 0xE000:
		
		switch( m_AudioAddress )
		{
		case 0x00:	// CH1 Fine Tune
			m_CH1Period = (m_CH1Period & 0xF00) | ch;
			break ;
		case 0x01:	// CH1 Course Tune
			m_CH1Period = (m_CH1Period & 0x0FF) | ((ch & 0xF)<<8);
			break ;
		case 0x02:	// CH2 Fine Tune
			m_CH2Period = (m_CH2Period & 0xF00) | ch;
			break ;
		case 0x03:	// CH2 Course Tune
			m_CH2Period = (m_CH2Period & 0x0FF) | ((ch & 0xF)<<8);
			break ;
		case 0x04:	// CH3 Fine Tune
			m_CH3Period = (m_CH3Period & 0xF00) | ch;
			break ;
		case 0x05:	// CH3 Course Tune
			m_CH3Period = (m_CH3Period & 0x0FF) | ((ch & 0xF)<<8);
			break ;
		case 0x06:	// Noise Period
			// TODO
			break ;
		case 0x07:	// Voice Enable
			m_CH1Enable = ~ch & 0x01;
			m_CH2Enable = ~ch & 0x02;
			m_CH3Enable = ~ch & 0x04;
			m_CH1Noise	= ~ch & 0x08;
			m_CH2Noise	= ~ch & 0x10;
			m_CH3Noise	= ~ch & 0x20;
			break ;
		case 0x08:	// CH1 Volume
			m_CH1Volume = ch & 0xF;
			// TODO: M Flag
			break ;
		case 0x09:	// CH2 Volume
			m_CH2Volume = ch & 0xF;
			// TODO: M Flag
			break ;
		case 0x0A:	// CH3 Volume
			m_CH3Volume = ch & 0xF;
			// TODO: M Flag
			break ;
		case 0x0B:	// Envelope Fine Tune
			// TODO
			break ;
		case 0x0C:	// Enveolpe Course Tune
			// TODO
			break ;
		case 0x0D:	// Envelope Shape
			// TODO
			break ;
		}

		break ;
	case 0xA000:
		switch( m_BankRegister )
		{
		case 0x0:	// VROM 0
			m_CHRBank0 = ((ch << 10) & m_CHRMask) + CHRRom;
			break ;
		case 0x1:	// VROM 1
			m_CHRBank1 = ((ch << 10) & m_CHRMask) + CHRRom;
			break ;
		case 0x2:	// VROM 2
			m_CHRBank2 = ((ch << 10) & m_CHRMask) + CHRRom;
			break ;
		case 0x3:	// VROM 3
			m_CHRBank3 = ((ch << 10) & m_CHRMask) + CHRRom;
			break ;
		case 0x4:	// VROM 4
			m_CHRBank4 = ((ch << 10) & m_CHRMask) + CHRRom;
			break ;
		case 0x5:	// VROM 5
			m_CHRBank5 = ((ch << 10) & m_CHRMask) + CHRRom;
			break ;
		case 0x6:	// VROM 6
			m_CHRBank6 = ((ch << 10) & m_CHRMask) + CHRRom;
			break ;
		case 0x7:	// VROM 7
			m_CHRBank7 = ((ch << 10) & m_CHRMask) + CHRRom;
			break ;
		case 0x8:
			switch( ch & 0xC0 )
			{
			case 0x00:	// RAM
			case 0x40:
				m_PRGBank0 = PRGRom + (((ch & 0x3F) << 13) & m_PRGMask);
				m_BankWritable = false;
				break ;
			case 0x80:
				// UNKNOWN MEMORY SPACE
				m_BankWritable = false;
				break ;
			case 0xC0:
				m_PRGBank0 = BCKRam;
				m_BankWritable = true;
				break ;
			}
			break ;
		case 0x9:
			m_PRGBank1 = ((ch  << 13) & m_PRGMask) + PRGRom;
			break ;
		case 0xA:
			m_PRGBank2 = ((ch  << 13) & m_PRGMask) + PRGRom;
			break ;
		case 0xB:
			m_PRGBank3 = ((ch  << 13) & m_PRGMask) + PRGRom;
			break ;
		case 0xC:
			switch( ch & 3 )
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
		case 0xD:
			m_IRQActive = false;
			m_IRQEnable = (ch == 0x81);
			m_Counter = m_CounterHolding;
			break ;
		case 0xE:
			m_CounterHolding = (m_CounterHolding & 0xFF00) | ch;
			break ;
		case 0xF:
			m_CounterHolding = (m_CounterHolding & 0x00FF) | (ch << 8);
			break ;
		}
		break ;
	}
}

void Sunsoft5::VideoAddressLatch( unsigned short Address )
{
	m_VideoAddress = Address;
}

void Sunsoft5::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		CHRRam[m_VideoAddress&0x1FFF] = Byte;
		break ;
	default:	// Name tables
		SetNameTable( m_VideoAddress, Byte );
		break ;
	}
}

unsigned char Sunsoft5::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3C00 )
	{
	case 0x0000:	// CHR-ROM
		return m_CHRBank0[(m_VideoAddress&0x3FF)];
	case 0x0400:	
		return m_CHRBank1[(m_VideoAddress&0x3FF)];
	case 0x0800:	
		return m_CHRBank2[(m_VideoAddress&0x3FF)];
	case 0x0C00:
		return m_CHRBank3[(m_VideoAddress&0x3FF)];
	case 0x1000:
		return m_CHRBank4[(m_VideoAddress&0x3FF)];
	case 0x1400:	
		return m_CHRBank5[(m_VideoAddress&0x3FF)];
	case 0x1800:
		return m_CHRBank6[(m_VideoAddress&0x3FF)];
	case 0x1C00:
		return m_CHRBank7[(m_VideoAddress&0x3FF)];
	default:	// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
