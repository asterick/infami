#include <memory.h>

#include "../Emulation/Emulation.h"
#include "NROM.h"
#include "CPROM.h"

void CPROM::Initalize( int Mirroring )
{
	NROM::Initalize( Mirroring );

	m_CHRMask = (CHRSize - 1) & ~0xFFF;
	m_Page = 0;
}

void CPROM::Write( unsigned short addr, unsigned char ch )
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
		m_Page = (ch & 3) << 12;
		break ;
	}
}

void CPROM::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
		CHRRom[m_VideoAddress&0x0FFF] = Byte;
		break ;
	case 0x1000:	// 
		CHRRom[(m_VideoAddress&0x0FFF)|m_Page] = Byte;
		break ;
	default:		// Name tables
		SetNameTable( m_VideoAddress, Byte );
		break ;
	}
}

unsigned char CPROM::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
		return CHRRom[m_VideoAddress&0x0FFF];
	case 0x1000:	// 
		return CHRRom[(m_VideoAddress&0x0FFF)|m_Page];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
