#include <memory.h>

#include "../Emulation/Emulation.h"
#include "Rambo1.h"

void Rambo1::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	m_PRGMask = PrgMask & ~0x1FFF;
	m_CHRMask = (CHRSize - 1) & ~0x3FF;

	m_CHRPage0 = 0x0000;
	m_CHRPage1 = 0x0400;
	m_CHRPage2 = 0x0800;
	m_CHRPage3 = 0x0C00;
	m_CHRPage4 = 0x1000;
	m_CHRPage5 = 0x1400;
	m_CHRPage6 = 0x1800;
	m_CHRPage7 = 0x1C00;

	m_PRGPage0 = m_PRGMask;
	m_PRGPage1 = m_PRGMask;
	m_PRGPage2 = m_PRGMask;

	m_Bank[0x0] = 0;
	m_Bank[0x8] = 1;
	m_Bank[0x1] = 2;
	m_Bank[0x9] = 3;
	m_Bank[0x2] = 4;
	m_Bank[0x3] = 5;
	m_Bank[0x4] = 6;
	m_Bank[0x5] = 7;

	m_Bank[0x6] = 0xFF;
	m_Bank[0x7] = 0xFF;
	m_Bank[0xF] = 0xFF;

	Mirroring = 0;
	m_ClockDivide = 0;
}

bool Rambo1::IRQActive()
{
	return m_IRQEnable && m_IRQActive;
}

void Rambo1::ClockIRQ()
{
	if( m_ReloadIRQ || m_IRQCounter == 0 )
	{
		m_ReloadIRQ = false;
		m_IRQCounter = m_IRQReload;
	}
	else
	{
		m_IRQCounter--;
	}

	if( m_IRQCounter == 0 && m_IRQEnable )
		m_IRQActive = true;
}

void Rambo1::CPUClock()
{
	if( !m_CPUClockIRQ || --m_ClockDivide  > 0)
		return ;

	m_ClockDivide = 4;
	ClockIRQ();
}

void Rambo1::ClockVideo( int edge )
{
	if( m_CPUClockIRQ )
		return ;

	if( !edge )
	{
		if( !(--m_EdgeTimeout) )
			m_A12Edge = false;

		return ;
	}
	else if( edge )
	{
		m_EdgeTimeout = 32;

		if( m_A12Edge )
			return ;

		m_A12Edge = true;
	}


	ClockIRQ();
}

unsigned char Rambo1::Read( unsigned short addr )
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
		return m_BusTrash = PRGRom[(addr & 0x1FFF) | m_PRGPage0];
	case 0xA000:
		return m_BusTrash = PRGRom[(addr & 0x1FFF) | m_PRGPage1];
	case 0xC000:
		return m_BusTrash = PRGRom[(addr & 0x1FFF) | m_PRGPage2];
	case 0xE000:
		return m_BusTrash = PRGRom[addr | m_PRGMask];
	}

	return m_BusTrash;
}

void Rambo1::BuildPages()
{
	if( m_PRGFlip )
	{
		m_PRGPage0 = (m_Bank[0x0F] << 13) & m_PRGMask;
		m_PRGPage1 = (m_Bank[0x06] << 13) & m_PRGMask;
		m_PRGPage2 = (m_Bank[0x07] << 13) & m_PRGMask;
	}
	else
	{
		m_PRGPage0 = (m_Bank[0x06] << 13) & m_PRGMask;
		m_PRGPage1 = (m_Bank[0x07] << 13) & m_PRGMask;
		m_PRGPage2 = (m_Bank[0x0F] << 13) & m_PRGMask;
	}

	if( m_CHRFlip )
	{
		m_CHRPage0 = (m_Bank[2] << 10) & m_CHRMask;
		m_CHRPage1 = (m_Bank[3] << 10) & m_CHRMask;
		m_CHRPage2 = (m_Bank[4] << 10) & m_CHRMask;
		m_CHRPage3 = (m_Bank[5] << 10) & m_CHRMask;

		if( m_1KMode )
		{
			m_CHRPage4 = (m_Bank[0] << 10) & m_CHRMask;
			m_CHRPage5 = (m_Bank[8] << 10) & m_CHRMask;
			m_CHRPage6 = (m_Bank[1] << 10) & m_CHRMask;
			m_CHRPage7 = (m_Bank[9] << 10) & m_CHRMask;
		}
		else
		{
			m_CHRPage4 = ((m_Bank[0] & 0xFE) << 10) & m_CHRMask;
			m_CHRPage5 = m_CHRPage4 + 0x400;
			m_CHRPage6 = ((m_Bank[1] & 0xFE) << 10) & m_CHRMask;
			m_CHRPage7 = m_CHRPage6 + 0x400;
		}

	}
	else
	{
		if( m_1KMode )
		{
			m_CHRPage0 = (m_Bank[0] << 10) & m_CHRMask;
			m_CHRPage1 = (m_Bank[8] << 10) & m_CHRMask;
			m_CHRPage2 = (m_Bank[1] << 10) & m_CHRMask;
			m_CHRPage3 = (m_Bank[9] << 10) & m_CHRMask;
		}
		else
		{
			m_CHRPage0 = ((m_Bank[0] & 0xFE) << 10) & m_CHRMask;
			m_CHRPage1 = m_CHRPage0 + 0x400;
			m_CHRPage2 = ((m_Bank[1] & 0xFE) << 10) & m_CHRMask;
			m_CHRPage3 = m_CHRPage2 + 0x400;
		}

		m_CHRPage4 = (m_Bank[2] << 10) & m_CHRMask;
		m_CHRPage5 = (m_Bank[3] << 10) & m_CHRMask;
		m_CHRPage6 = (m_Bank[4] << 10) & m_CHRMask;
		m_CHRPage7 = (m_Bank[5] << 10) & m_CHRMask;
	}
}

void Rambo1::Write( unsigned short addr, unsigned char ch )
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
	case 0x8000:
		if( addr & 0x0001 )
		{
			m_Bank[m_BankSelect] = ch;
		}
		else
		{
			m_BankSelect = ch & 0x0F;
			m_CHRFlip	 = ch & 0x80;
			m_PRGFlip	 = ch & 0x40;
			m_1KMode	 = ch & 0x20;
		}
		BuildPages();

		break ;
	case 0xA000:
		if( ~addr & 0x0001 )
		{
			if( ch & 1 )
				SetMirrorVertical();
			else
				SetMirrorHorizontal();
		}
		break ;
	case 0xC000:
		if( addr & 0x0001 )
		{
			m_ReloadIRQ = true;
			m_CPUClockIRQ = (ch & 1) ? true : false;
		}
		else
		{
			m_IRQReload = ch;
		}
		break ;
	case 0xE000:
		if( addr & 1 )
			m_IRQEnable = true;
		else
			m_IRQActive = m_IRQEnable = false;
		break ;
	}
}

void Rambo1::VideoAddressLatch( unsigned short Address )
{
	ClockVideo(Address & 0x1000);
	m_VideoAddress = Address;
}

void Rambo1::VideoWrite( unsigned char Byte )
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

unsigned char Rambo1::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3C00 )
	{
	case 0x0000:	// CHR-ROM
		return CHRRom[(m_VideoAddress&0x3FF)|m_CHRPage0];
	case 0x0400:	
		return CHRRom[(m_VideoAddress&0x3FF)|m_CHRPage1];
	case 0x0800:	
		return CHRRom[(m_VideoAddress&0x3FF)|m_CHRPage2];
	case 0x0C00:
		return CHRRom[(m_VideoAddress&0x3FF)|m_CHRPage3];
	case 0x1000:
		return CHRRom[(m_VideoAddress&0x3FF)|m_CHRPage4];
	case 0x1400:	
		return CHRRom[(m_VideoAddress&0x3FF)|m_CHRPage5];
	case 0x1800:
		return CHRRom[(m_VideoAddress&0x3FF)|m_CHRPage6];
	case 0x1C00:
		return CHRRom[(m_VideoAddress&0x3FF)|m_CHRPage7];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
