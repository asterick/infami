#include <memory.h>

#include "../Emulation/Emulation.h"
#include "VRC1.h"

void VRC1::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );
	Mirroring = 0;

	m_PRGMask = (PRGSize-1) & ~0x1FFF;
	m_CHRMask = (CHRSize-1) & ~0x0FFF;

	m_PRGPage0 = 0;
	m_PRGPage1 = 0;
	m_PRGPage2 = 0;

	m_CHRPage0 = 0;
	m_CHRPage1 = 0;
}

unsigned char VRC1::Read( unsigned short addr )
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
		return m_BusTrash = PRGRom[(addr&0x1FFF) | m_PRGPage0];
	case 0xA000:
		return m_BusTrash = PRGRom[(addr&0x1FFF) | m_PRGPage1];
	case 0xC000:
		return m_BusTrash = PRGRom[(addr&0x1FFF) | m_PRGPage2];
	case 0xE000:
		return m_BusTrash = PRGRom[addr | m_PRGMask];
	}

	return m_BusTrash;
}

void VRC1::Write( unsigned short addr, unsigned char ch )
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
		case 0x8000:	// PRG0
			m_PRGPage0 = (m_BusTrash << 13) & m_PRGMask;
			break ;
		case 0xA000:	// PRG1
			m_PRGPage1 = (m_BusTrash << 13) & m_PRGMask;
			break ;
		case 0xC000:	// PRG2
			m_PRGPage2 = (m_BusTrash << 13) & m_PRGMask;
			break ;

		case 0x9000:	// MIRRORING CHR(0/1).5
			m_CHRPage0 = ((m_CHRPage0 & 0x0FFFF) | ((m_BusTrash << 15)& 0x10000)) & m_CHRMask;
			m_CHRPage1 = ((m_CHRPage1 & 0x0FFFF) | ((m_BusTrash << 14)& 0x10000)) & m_CHRMask;
			
			if( m_BusTrash & 1 )
				SetMirrorHorizontal();
			else
				SetMirrorVertical();
			break ;
		case 0xE000:	// CHR0
			m_CHRPage0 = ((m_CHRPage0 & 0x10000) | (m_BusTrash << 12)) & m_CHRMask;
			break ;
		case 0xF000:	// CHR1
			m_CHRPage1 = ((m_CHRPage1 & 0x10000) | (m_BusTrash << 12)) & m_CHRMask;
			break ;
		}
	}
}

void VRC1::VideoAddressLatch( unsigned short Address )
{
	m_VideoAddress = Address;
}

void VRC1::VideoWrite( unsigned char Byte )
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

unsigned char VRC1::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
		return CHRRom[(m_VideoAddress&0xFFF)|m_CHRPage0];
	case 0x1000:	// 
		return CHRRom[(m_VideoAddress&0xFFF)|m_CHRPage1];
	default:	// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
