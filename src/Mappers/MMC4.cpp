#include <memory.h>

#include "../Emulation/Emulation.h"
#include "MMC4.h"

void MMC4::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	m_PRGMask	= (PRGSize - 1) & ~0x3FFF;
	m_CHRMask	= (CHRSize - 1) & ~0x0FFF;

	m_ChrLoLatch	= true;
	m_ChrHiLatch	= true;

	m_PRGPage	= 0;
	m_CHRPage0	= 0;
	m_CHRPage1	= 0;
	m_CHRPage2	= 0;
	m_CHRPage3	= 0;

	Mirroring	= 0;
}

unsigned char MMC4::Read( unsigned short addr )
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
		return m_BusTrash = PRGRom[(addr & 0x3FFF) | m_PRGPage ];
	case 0xC000:
	case 0xE000:
		return m_BusTrash = PRGRom[addr | m_PRGMask];
	}

	return m_BusTrash;
}

void MMC4::Write( unsigned short addr, unsigned char ch )
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
		switch( addr & 0xF000 )
		{
			case 0xA000:
				m_PRGPage = (ch << 14) & m_PRGMask;
				break;
			case 0xB000:
				m_CHRPage0 = (ch << 12) & m_CHRMask;
				break ;
			case 0xC000:
				m_CHRPage1 = (ch << 12) & m_CHRMask;
				break ;
			case 0xD000:
				m_CHRPage2 = (ch << 12) & m_CHRMask;
				break ;
			case 0xE000:
				m_CHRPage3 = (ch << 12) & m_CHRMask;
				break ;
			case 0xF000:
				if( ch & 1 )
					SetMirrorHorizontal();
				else
					SetMirrorVertical();
				break ;
		}
	}
}

void MMC4::VideoAddressLatch( unsigned short Address ) 
{
	m_VideoAddress = Address;

	switch( m_VideoAddress & 0x1FF0 )
	{
	case 0x0FD0:
		m_ChrLoLatch = false;
		break ;
	case 0x0FE0:
		m_ChrLoLatch = true;
		break ;
	case 0x1FD0:
		m_ChrHiLatch = false;
		break ;
	case 0x1FE0:
		m_ChrHiLatch = true;
		break ;
	}
}

void MMC4::VideoWrite( unsigned char Byte )
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

unsigned char MMC4::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
		return CHRRom[(m_VideoAddress&0x0FFF) | (m_ChrLoLatch ? m_CHRPage1 : m_CHRPage0) ];
	case 0x1000:	// 
		return CHRRom[(m_VideoAddress&0x0FFF) | (m_ChrHiLatch ? m_CHRPage3 : m_CHRPage2) ];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
