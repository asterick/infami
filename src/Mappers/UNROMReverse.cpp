#include <memory.h>

#include "../Emulation/Emulation.h"
#include "NROM.h"
#include "UNROMReverse.h"

void UNROMReverse::Initalize( int Mirroring )
{	
	NROM::Initalize( Mirroring );

	m_Page		= PRGRom;
	m_PageMask	= PrgMask;
	m_TopPage	= m_PageMask & 0x3C000;
}

unsigned char UNROMReverse::Read( unsigned short addr )
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
		return m_BusTrash = m_Page[ addr & 0x3FFF ];
	case 0xC000:
	case 0xE000:
		return m_BusTrash = PRGRom[ addr | m_TopPage ];
	}

	return m_BusTrash;
}

void UNROMReverse::Write( unsigned short addr, unsigned char ch )
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
		m_Page = (((ch^0xF8) << 14) & m_PageMask) + PRGRom;
		break ;
	}
}