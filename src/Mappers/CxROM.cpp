#include <memory.h>

#include "../Emulation/Emulation.h"
#include "NROM.h"
#include "CXROM.h"

void CxROM::Initalize( int Mirroring )
{
	NROM::Initalize( Mirroring );

	m_CHRMask = (CHRSize - 1) & ~0x1FFF;
	m_Page = 0;
}

void CxROM::Write( unsigned short addr, unsigned char ch )
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
		m_Page = ((ch & 3) << 13) & m_CHRMask;
		break ;
	}
}

unsigned char CxROM::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		return CHRRom[(m_VideoAddress&0x1FFF)|m_Page];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
