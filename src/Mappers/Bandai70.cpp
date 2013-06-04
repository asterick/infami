#include <memory.h>

#include "../Emulation/Emulation.h"
#include "Bandai70.h"

void Bandai70::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	PrgMask = (PRGSize - 1) & ~0x3FFF;
	ChrMask = (CHRSize - 1) & ~0x1FFF;

	m_CHRBank = 0;
	m_PRGBank = 0;
	m_NameTable = 0;
}

unsigned char Bandai70::Read( unsigned short addr )
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
		return m_BusTrash = PRGRom[(addr & 0x3FFF) | m_PRGBank ];
	case 0xC000:
	case 0xE000:
		return m_BusTrash = PRGRom[addr | PrgMask];
	}

	return m_BusTrash;
}

void Bandai70::Write( unsigned short addr, unsigned char ch )
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
	case 0x8000:
	case 0xA000:
	case 0xC000:
	case 0xE000:
		m_CHRBank = ((ch & 0xF) << 13) & ChrMask;
		m_PRGBank = ((ch & 0x70) << 10) & PrgMask;

		if( ch & 80 )
			SetMirrorOne();
		else
			SetMirrorZero();

		break ;
	}
}

void Bandai70::VideoAddressLatch( unsigned short Address ) 
{
	m_VideoAddress = Address; 
}

void Bandai70::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		CHRRam[(m_VideoAddress & 0x1FFF) | m_CHRBank] = Byte;
		break ;
	default:		// Name tables
		SetNameTable( m_VideoAddress, Byte );
		break ;
	}
}

unsigned char Bandai70::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		return CHRRom[(m_VideoAddress & 0x1FFF) | m_CHRBank];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
