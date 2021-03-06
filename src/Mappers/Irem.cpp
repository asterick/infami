#include "../Emulation/Emulation.h"
#include "NROM.h"
#include "Irem.h"

void Irem::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	m_PRGMask = PrgMask & ~0x7FFF;
	m_CHRMask = (CHRSize - 1) & ~0x07FF;

	m_PRGPage = 0;
	m_CHRPage = 0;
}

unsigned char Irem::Read( unsigned short addr )
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
			return m_BusTrash = BCKRam[ addr & RamMask ] ;
		return m_BusTrash;
	case 0x8000:
	case 0xA000:
	case 0xC000:
	case 0xE000:
		return m_BusTrash = PRGRom[(addr & PrgMask) | m_PRGPage];
	}

	return m_BusTrash;
}

void Irem::Write( unsigned short addr, unsigned char ch )
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
			BCKRam[ addr & RamMask ] = m_BusTrash;
		break ;
	case 0x8000:
	case 0xA000:
	case 0xC000:
	case 0xE000:
		m_PRGPage = (m_BusTrash << 15) & m_PRGMask;
		m_CHRPage = ((m_BusTrash & 0xF0) << 7) & m_CHRMask;
		break ;
	}
}

void Irem::VideoAddressLatch( unsigned short Address )
{
	m_VideoAddress = Address;
}

void Irem::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		CHRRam[m_VideoAddress & 0x1FFF] = Byte;
		break ;
	default:	// Name tables
		SetNameTable( m_VideoAddress, Byte );
		break ;
	}
}

unsigned char Irem::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
		if( !(m_VideoAddress & 0x1800) )
			return CHRRom[(m_VideoAddress&0x07FF) | m_CHRPage];
	case 0x1000:	// 
		return CHRRam[m_VideoAddress & 0x1FFF];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
