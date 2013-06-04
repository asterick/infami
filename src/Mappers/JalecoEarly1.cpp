#include <memory.h>

#include "../Emulation/Emulation.h"
#include "JalecoEarly1.h"

void JalecoEarly1::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	m_PRGMask = PrgMask & ~0x3FFF;
	m_CHRMask = (CHRSize - 1) & ~0x1FFF;

	m_CHRLatch = 0;
	m_PRGLatch = 0;//(0xF << 14) & m_PRGMask;
}

unsigned char JalecoEarly1::Read( unsigned short addr )
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
		return m_BusTrash = PRGRom[addr & 0x3FFF];
	case 0xC000:
	case 0xE000:
		return m_BusTrash = PRGRom[(addr & 0x3FFF) | m_PRGLatch];
	}

	return m_BusTrash;
}

void JalecoEarly1::Write( unsigned short addr, unsigned char ch )
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
		{
			int select = (ch ^ m_HoldingLatch) & ~ch & 0xC0;

			if( select & 0x40 )	// VROM LATCH
				m_CHRLatch = ((m_HoldingLatch & 0xF) << 13) & m_CHRMask;			
			if( select & 0x80 )	// ROM LATCH
				m_PRGLatch = ((m_HoldingLatch & 0xF) << 14) & m_PRGMask;
		}
		
		m_HoldingLatch = ch;
	}
}

void JalecoEarly1::VideoAddressLatch( unsigned short Address )
{
	m_VideoAddress = Address;
}

void JalecoEarly1::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		CHRRam[(m_VideoAddress&0x1FFF)|m_CHRLatch] = Byte;
		break ;
	default:		// Name tables
		SetNameTable( m_VideoAddress, Byte );
		break ;
	}
}

unsigned char JalecoEarly1::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		return CHRRom[(m_VideoAddress&0x1FFF)|m_CHRLatch];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
