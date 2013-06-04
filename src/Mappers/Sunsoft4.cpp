#include <memory.h>

#include "../Emulation/Emulation.h"
#include "Sunsoft4.h"

void Sunsoft4::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	Mirroring = Vertical;

	m_PRGMask = PrgMask & ~0x3FFF;
	m_CHRMask = (CHRSize - 1) & ~0x07FF;
	m_VROMMask = (CHRSize - 1) & ~0x03FF;

	m_PRGBank = 0;
	m_CHRBank0 = 0;
	m_CHRBank1 = 0;
	m_CHRBank2 = 0;
	m_CHRBank3 = 0;
	m_VROMBank0 = 0;
	m_VROMBank1 = 0;
}

unsigned char Sunsoft4::Read( unsigned short addr )
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
		return m_BusTrash = PRGRom[addr | m_PRGMask];
	}

	return m_BusTrash;
}

void Sunsoft4::SetNameTables()
{
	switch( Mirroring )
	{
	case 1:
		if( m_VROMMode )
		{
			SetMirrorPage( 0, m_VROMBank0 + CHRRom );
			SetMirrorPage( 1, m_VROMBank0 + CHRRom );
			SetMirrorPage( 2, m_VROMBank1 + CHRRom );
			SetMirrorPage( 3, m_VROMBank1 + CHRRom );
		}
		else
		{
			SetMirrorHorizontal();
		}
		break ;
	case 0:
		if( m_VROMMode )
		{
			SetMirrorPage( 0, m_VROMBank0 + CHRRom );
			SetMirrorPage( 1, m_VROMBank1 + CHRRom );
			SetMirrorPage( 2, m_VROMBank0 + CHRRom );
			SetMirrorPage( 3, m_VROMBank1 + CHRRom );
		}
		else
		{
			SetMirrorVertical();
		}
		break ;
	}
}

void Sunsoft4::Write( unsigned short addr, unsigned char ch )
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

	default:
		switch( addr & 0xF000 )
		{
		case 0x8000:
			m_CHRBank0 = (m_BusTrash << 11) & m_CHRMask;
			break ;
		case 0x9000:
			m_CHRBank1 = (m_BusTrash << 11) & m_CHRMask;
			break ;
		case 0xA000:
			m_CHRBank2 = (m_BusTrash << 11) & m_CHRMask;
			break ;
		case 0xB000:
			m_CHRBank3 = (m_BusTrash << 11) & m_CHRMask;
			break ;
		case 0xC000:
			m_VROMBank0 = ((m_BusTrash | 0x80) << 10) & m_VROMMask;
			SetNameTables();
			break ;
		case 0xD000:
			m_VROMBank1 = ((m_BusTrash | 0x80) << 10) & m_VROMMask;
			SetNameTables();
			break ;
		case 0xE000:
			m_VROMMode = (ch & 0x10) ? true : false;
			Mirroring = ch & 1;
			SetNameTables();

			break ;
		case 0xF000:
			m_PRGBank = (ch << 14) & m_PRGMask;
			break ;
		}
		break ;
	}
}

void Sunsoft4::VideoAddressLatch( unsigned short Address )
{
	m_VideoAddress = Address;
}

void Sunsoft4::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		CHRRam[m_VideoAddress&0x1FFF] = Byte;
		break ;
	default:	// Name tables
		if( m_VROMMode )
			break ;

		SetNameTable( m_VideoAddress, Byte );
		break ;
	}
}

unsigned char Sunsoft4::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3800 )
	{
	case 0x0000:	// CHR-ROM
		return CHRRom[(m_VideoAddress&0x7FF)|m_CHRBank0];
	case 0x0800:	
		return CHRRom[(m_VideoAddress&0x7FF)|m_CHRBank1];
	case 0x1000:	
		return CHRRom[(m_VideoAddress&0x7FF)|m_CHRBank2];
	case 0x1800:
		return CHRRom[(m_VideoAddress&0x7FF)|m_CHRBank3];
	default:	// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
