#include <memory.h>

#include "../Emulation/Emulation.h"
#include "NROM.h"
#include "VROMProtect.h"

void VROMProtect::Initalize( int Mirroring )
{
	NROM::Initalize( Mirroring );

	m_Disable = false;
}

void VROMProtect::Write( unsigned short addr, unsigned char ch )
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
		m_Disable = (ch == 0x13) || !(ch & 0x0F);
		break ;
	}
}

unsigned char VROMProtect::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		if( m_Disable )
			return 0;
		return CHRRom[m_VideoAddress&0x1FFF];
	default:		// Name tables
		return GetNameTable(m_VideoAddress);
	}
	return 0; 
}
