#include <memory.h>

#include "../Emulation/Emulation.h"
#include "NROM.h"
#include "GxROM.h"

void GxROM::Initalize( int Mirroring )
{
	NROM::Initalize( Mirroring );

	m_PRGMask = PrgMask & 0x18000;
	m_CHRMask = (CHRSize - 1) & 0x06000;
	m_PRGBank = 0;
	m_CHRBank = 0;
}

unsigned char GxROM::Read( unsigned short addr )
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
	case 0xC000:
	case 0xE000:
		return m_BusTrash = PRGRom[(addr & 0x7FFF) | m_PRGBank];
	}

	return m_BusTrash;
}

void GxROM::Write( unsigned short addr, unsigned char ch )
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
	case 0xA000:
	case 0xC000:
	case 0xE000:
		m_PRGBank = (ch << 11) & m_PRGMask;
		m_CHRBank = (ch << 13) & m_CHRMask;
		break ;
	}
}

unsigned char GxROM::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		return CHRRom[(m_VideoAddress&0x1FFF) | m_CHRBank];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
