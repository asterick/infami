#include <memory.h>

#include "../Emulation/Emulation.h"
#include "NROM.h"
#include "Camerica.h"

void Camerica::Initalize( int Mirroring )
{
	NROM::Initalize( Mirroring );

	m_PRGMask = PrgMask & ~0x3FFF;
	m_PRGBank = 0;
	m_NamePage = 0;
}

unsigned char Camerica::Read( unsigned short addr )
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
		return m_BusTrash = PRGRom[(addr & 0x3FFF) | m_PRGBank];
	case 0xC000:
	case 0xE000:
		return m_BusTrash = PRGRom[ addr | m_PRGMask];
	}

	return m_BusTrash;
}

void Camerica::Write( unsigned short addr, unsigned char ch )
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
		if( addr & 0x1000 )
		{
			if( ch & 0x10 )
				SetMirrorOne();
			else
				SetMirrorZero();
		}	
		break ;
	case 0xC000:
	case 0xE000:
		m_PRGBank = (ch << 14) & m_PRGMask;
		break ;
	}
}

void Camerica::VideoWrite( unsigned char Byte )
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

unsigned char Camerica::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-RAM
	case 0x1000:	// 
		return CHRRom[m_VideoAddress&0x1FFF];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
}
