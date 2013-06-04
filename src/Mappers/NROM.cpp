#include <memory.h>

#include "../Emulation/Emulation.h"
#include "NROM.h"

unsigned char NROM::Read( unsigned short addr )
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
		return m_BusTrash = PRGRom[addr & PrgMask];
	}

	return m_BusTrash;
}

void NROM::Write( unsigned short addr, unsigned char ch )
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
	}
}

void NROM::VideoAddressLatch( unsigned short Address ) 
{
	m_VideoAddress = Address; 
}

void NROM::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x2000 )
	{
	case 0x0000:	// CHR-ROM
		CHRRam[m_VideoAddress&0x1FFF] = Byte;
		break ;
	default:		// Name tables
		SetNameTable( m_VideoAddress, Byte );
		break ;
	}
}

unsigned char NROM::VideoRead() 
{ 
	switch( m_VideoAddress & 0x2000 )
	{
	case 0x0000:	// CHR-ROM
		return CHRRom[m_VideoAddress&0x1FFF];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
