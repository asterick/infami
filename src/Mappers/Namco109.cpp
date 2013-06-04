#include "../Emulation/Emulation.h"
#include "NROM.h"
#include "Namco109.h"

void Namco109::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	m_PRGMask = PrgMask & ~0x1FFF;
	m_CHRMask = (CHRSize - 1) & ~0x07FF;

	m_PageSelect = 0;
	m_PRGPage0 = 0;
	m_PRGPage1 = 0;
	m_CHRPage0 = 0;
	m_CHRPage1 = 0;
	m_CHRPage2 = 0;
	m_CHRPage3 = 0;
}

unsigned char Namco109::Read( unsigned short addr )
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
			return m_BusTrash = BCKRam[ addr & RamMask ] ;
		return m_BusTrash;
	case 0x8000:
		return m_BusTrash = PRGRom[(addr & 0x1FFF) | m_PRGPage0];
	case 0xA000:
		return m_BusTrash = PRGRom[(addr & 0x1FFF) | m_PRGPage1];
	case 0xC000:
	case 0xE000:
		return m_BusTrash = PRGRom[(addr & 0x3FFF) | (m_PRGMask & ~0x3FFF)];
	}

	return m_BusTrash;
}

void Namco109::Write( unsigned short addr, unsigned char ch )
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
			BCKRam[ addr & RamMask ] = m_BusTrash;
		break ;
	case 0x8000:
		if( addr == 0x8000 )
		{
			m_PageSelect = m_BusTrash & 0x7;
		}
		else if( addr == 0x8001 )
		{
			switch( m_PageSelect )
			{
			case 2:
				m_CHRPage0 = (m_BusTrash << 11) & m_CHRMask;
				break ;
			case 3:
				m_CHRPage1 = (m_BusTrash << 11) & m_CHRMask;
				break ;
			case 4:
				m_CHRPage2 = (m_BusTrash << 11) & m_CHRMask;
				break ;
			case 5:
				m_CHRPage3 = (m_BusTrash << 11) & m_CHRMask;
				break ;
			case 6:
				m_PRGPage0 = (m_BusTrash << 13) & m_PRGMask;
				break ;
			case 7:
				m_PRGPage1 = (m_BusTrash << 13) & m_PRGMask;
				break ;
			}
		}
		break ;	
	case 0xA000:
	case 0xC000:
	case 0xE000:
		break ;
	}
}

void Namco109::VideoAddressLatch( unsigned short Address )
{
	m_VideoAddress = Address;
}

void Namco109::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		CHRRam[m_VideoAddress & 0x1FFF] = Byte;
		break ;
	default:		// Name tables
		SetNameTable( m_VideoAddress, Byte );
		break ;
	}
}

unsigned char Namco109::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
		if( m_VideoAddress & 0x0800 )
			return CHRRom[(m_VideoAddress&0x07FF) | m_CHRPage1];
		else
			return CHRRom[(m_VideoAddress&0x07FF) | m_CHRPage0];
	case 0x1000:	// 
		if( m_VideoAddress & 0x0800 )
			return CHRRom[(m_VideoAddress&0x07FF) | m_CHRPage3];
		else
			return CHRRom[(m_VideoAddress&0x07FF) | m_CHRPage2];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
