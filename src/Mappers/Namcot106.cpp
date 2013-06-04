#include "../Emulation/Emulation.h"
#include "Namcot106.h"

void Namcot106::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	PrgMask = (PRGSize - 1) & ~0x1FFF;
	ChrMask = (CHRSize - 1) & ~0x03FF;

	m_PRGPage0 = PRGRom;
	m_PRGPage1 = 0x2000 + PRGRom;
	m_PRGPage2 = ((0xFE << 13) & PrgMask) + PRGRom;

	m_LowPageRam = 0;
	m_HighPageRam = 0;

	m_IRQEnable = false;
	m_IRQActive = false;
	m_Counter	= 0;	

	m_AudioAddress = 0;

	for( int i = 0; i < 12; i++ )
	{
		m_CHRBank[i] = 0;
		m_CHRProtect[i] = true;
		m_CHRPage[i] = CHRRom;
	}

	// Initalize audio
	m_OutputChannel = 0;

	for( int i = 0; i < 0x80; i++ )
		m_AudioRam[i] = 0;

	for( int i = 0; i < 0x100; i++ )
		m_WaveTable[i] = 0;

	for( int i = 0; i < 8; i++ )
	{
		AudioUpdate( 0x40 + (i << 3) );

		m_WaveChannel[i].m_CurrentAddress = 0;
		m_WaveChannel[i].m_BytesLeft = 0;
		m_WaveChannel[i].m_Output = 0;
		m_WaveChannel[i].m_Error = 0;
	}
}

void Namcot106::AudioUpdate( int offset )
{
	offset &= 0xF8;
	int channel = 15 - (offset >> 3);

	m_WaveChannel[channel].m_Frequency = 
		  m_AudioRam[ offset + 0x00 ] | 
		 (m_AudioRam[ offset + 0x02 ] << 8) |
		((m_AudioRam[ offset + 0x04 ] & 0x3) << 16);

	m_WaveChannel[channel].m_WaveLength		= 32 - (m_AudioRam[ offset + 0x04 ] & 0x1C);
	m_WaveChannel[channel].m_WaveAddress	= m_AudioRam[ offset + 0x06 ];
	m_WaveChannel[channel].m_Volume			= m_AudioRam[ offset + 0x07 ] & 0xF;
	
	m_ActiveChannels = (m_AudioRam[ 0x7F ] >> 4) & 0x7;

	return ;
}

float Namcot106::StreamAudio()
{
	float output = m_AudioAccumulator / m_AudioSamples / 256.0f;

	output += ROM::StreamAudio();

	m_AudioSamples = 0;
	m_AudioAccumulator = 0;

	return output;
}

bool Namcot106::IRQActive()
{
	return m_IRQActive;
}

void Namcot106::CPUClock()
{
	// Accumulate wavetables
	m_AudioAccumulator += m_WaveChannel[m_OutputChannel].m_Output;
	m_AudioSamples++;

	// Sample audio
	if( ++m_OutputChannel > m_ActiveChannels )
		m_OutputChannel = 0;

	int i = m_OutputChannel;

	m_WaveChannel[i].m_Error += m_WaveChannel[i].m_Frequency;
	if( m_WaveChannel[i].m_Error > 0xF0000 )
	{
		if( --m_WaveChannel[i].m_BytesLeft <= 0 )
		{
			m_WaveChannel[i].m_CurrentAddress = m_WaveChannel[i].m_WaveAddress;
			m_WaveChannel[i].m_BytesLeft = m_WaveChannel[i].m_WaveLength;
		}
		else
		{
			m_WaveChannel[i].m_CurrentAddress++;
		}

		m_WaveChannel[i].m_Error -= 0xF0000;
		m_WaveChannel[i].m_Output = m_WaveChannel[i].m_Volume * m_WaveTable[ m_WaveChannel[i].m_CurrentAddress ];
	}

	// Process IRQs
	if( m_IRQEnable && (m_Counter != 0x7FFF) && (++m_Counter == 0x7FFF) )
		m_IRQActive = true;
}

unsigned char Namcot106::RegisterRead( unsigned short addr )
{
	switch( addr & 0xF800 )
	{
	case 0x4800:
		{
			unsigned char ch = m_AudioRam[m_AudioAddress];
			if( m_AudioIncrement )
				m_AudioAddress = (m_AudioAddress + 1) & 0x7F;
			return ch;
		}
		break ;
	case 0x5000:
		m_IRQActive = false;
		return m_BusTrash = m_Counter & 0xFF;
	case 0x5800:
		m_IRQActive = false;
		return m_BusTrash = (m_Counter >> 8) | (m_IRQEnable ? 0x80 : 0);
	}

	return 0;
}

void Namcot106::RegisterWrite( unsigned short addr, unsigned char ch )
{
	switch( addr & 0xF800 )
	{
	case 0x4800:
		m_AudioRam[m_AudioAddress] = ch;
		m_WaveTable[ (m_AudioAddress<<1) ] = (ch & 0xF) - 8;
		m_WaveTable[(m_AudioAddress<<1)+1] = (ch >> 4) - 8;
		
		if( m_AudioAddress >= 0x40 )
			AudioUpdate( m_AudioAddress );
		
		if( m_AudioIncrement )
			m_AudioAddress = (m_AudioAddress + 1) & 0x7F;
		break ;
	case 0x5000:
		m_IRQActive = false;
		m_Counter = (m_Counter & 0x7F00) | ch;
		break ;
	case 0x5800:
		m_IRQActive = false;
		m_IRQEnable = (ch & 0x80) != 0;
		m_Counter = (m_Counter & 0x00FF) | ((ch & 0x7F) << 8);
		break ;
	case 0x8000:
	case 0x8800:
	case 0x9000:
	case 0x9800:
	case 0xA000:
	case 0xA800:
	case 0xB000:
	case 0xB800:
	case 0xC000:
	case 0xC800:
	case 0xD000:
	case 0xD800:
		m_CHRBank[(addr >> 11) - 0x10] = ch;
		break ;
	case 0xE000:
		m_PRGPage0 = ((ch << 13) & PrgMask) + PRGRom;
		return ;
	case 0xE800:
		m_PRGPage1 = ((ch << 13) & PrgMask) + PRGRom;
		m_LowPageRam = !(ch & 0x40);
		m_HighPageRam = !(ch & 0x80);
		break ;
	case 0xF000:
		m_PRGPage2 = ((ch << 13) & PrgMask) + PRGRom;
		return ;
	case 0xF800:
		m_AudioIncrement = (ch & 0x80) != 0;
		m_AudioAddress = ch & 0x7F;		
		return ;
	default:
		return ;
	}

	for( int i = 0; i < 12; i++ )
	{
		if( m_CHRBank[i] >= 0xE0 && ( ((i<4) && m_LowPageRam) || ((i>=4) && m_HighPageRam) || (i >= 8)) )
		{
			m_CHRProtect[i] = false;

			int bank = m_CHRBank[i] - 0xE0;

			if( i >= 8 )
				m_CHRPage[i] = m_NameTables + ((bank << 10) & 0x400);
			else
				m_CHRPage[i] = CHRRam + ((bank << 10) & ChrMask);
		}
		else
		{
			m_CHRProtect[i] = true;
			m_CHRPage[i] = CHRRom + ((m_CHRBank[i] << 10) & ChrMask);
		}
	}
}

unsigned char Namcot106::Read( unsigned short addr )
{
	switch( addr & 0xE000 )
	{
	case 0x0000:
		return m_BusTrash = SystemRam[addr&0x7FF];
	case 0x2000:
		return m_BusTrash = PPURead( addr, m_BusTrash );
	case 0x4000:
		if( addr < 0x4800 )
			return m_BusTrash = APURead( addr, m_BusTrash );
		else
			return m_BusTrash = RegisterRead(addr);
	case 0x6000:
		if( BCKRam )
			return m_BusTrash = BCKRam[addr & 0x1FFF];
		break ;
	case 0x8000:
		return m_BusTrash = m_PRGPage0[addr & 0x1FFF];
	case 0xA000:
		return m_BusTrash = m_PRGPage1[addr & 0x1FFF];
	case 0xC000:
		return m_BusTrash = m_PRGPage2[addr & 0x1FFF];
	case 0xE000:
		return m_BusTrash = PRGRom[addr | PrgMask];
	}

	return m_BusTrash;
}

void Namcot106::Write( unsigned short addr, unsigned char ch )
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
		if( addr < 0x4800 )
			APUWrite( addr, m_BusTrash );		
		else
			RegisterWrite( addr, m_BusTrash );
		break ;
	case 0x6000:
		if( BCKRam )
			BCKRam[ addr & RamMask ] = ch;
		break ;
	default:
		RegisterWrite( addr, m_BusTrash );
		break ;
	}
}

void Namcot106::VideoAddressLatch( unsigned short Address ) 
{
	m_VideoAddress = Address;
}

void Namcot106::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:
		{
			int bank = (m_VideoAddress & 0x1C00) >> 10;
			if( !m_CHRProtect[bank] )
				m_CHRPage[bank][ m_VideoAddress & 0x3FF ] = Byte;
			break ;
		}
		break ;
	default:		// Name tables
		{
			int bank = (m_VideoAddress & 0x2C00) >> 10;
			if( !m_CHRProtect[bank] )
				m_CHRPage[bank][ m_VideoAddress & 0x3FF ] = Byte;
			break ;
		}
		break ;
	}
}

unsigned char Namcot106::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:
		{
			int bank = (m_VideoAddress & 0x1C00) >> 10;
			return m_CHRPage[bank][ m_VideoAddress & 0x3FF ];
		}
	default:		// Mirror nametable
		{
			int bank = (m_VideoAddress & 0x2C00) >> 10;
			return m_CHRPage[bank][m_VideoAddress & 0x3FF];
		}
	}
	return 0; 
}
