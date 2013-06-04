#include "../Emulation/Emulation.h"
#include "NROM.h"
#include "BxNINA.h"

void BxNINA::Initalize( int Mirroring ) 
{	
	ROM::Initalize( Mirroring );

	m_PRGMask = PrgMask & ~0x7FFF;
	m_CHRMask = (CHRSize - 1) & ~0x0FFF;

	m_PRGPage = m_PRGMask;
	m_CHR0Page = 0;
	m_CHR1Page = 0;
}

unsigned char BxNINA::Read( unsigned short addr )
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
		break ;
	case 0x8000:
	case 0xA000:
	case 0xC000:
	case 0xE000:
		return m_BusTrash = PRGRom[(addr & 0x7FFF) | m_PRGPage];
	}

	return m_BusTrash;
}

void BxNINA::Write( unsigned short addr, unsigned char ch )
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
		if( addr == 0x7FFD )
			m_PRGPage = (m_BusTrash << 15) & m_PRGMask;
		else if( addr == 0x7FFE )
			m_CHR0Page = (m_BusTrash << 12) & m_CHRMask;
		else if( addr == 0x7FFF )
			m_CHR1Page = (m_BusTrash << 12) & m_CHRMask;

		if( BCKRam )
			BCKRam[ addr & RamMask ] = m_BusTrash;
		break ;
	case 0x8000:
	case 0xA000:
	case 0xC000:
	case 0xE000:
		break ;
	}
}

void BxNINA::VideoAddressLatch( unsigned short Address )
{
	m_VideoAddress = Address;
}

void BxNINA::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
		CHRRam[(m_VideoAddress&0x0FFF) | m_CHR0Page] = Byte;
		break ;
	case 0x1000:	// 
		CHRRam[(m_VideoAddress&0x0FFF) | m_CHR1Page] = Byte;
		break ;
	default:		// Name tables
		SetNameTable( m_VideoAddress, Byte );
		break ;
	}
}

unsigned char BxNINA::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
		return CHRRom[(m_VideoAddress&0x0FFF) | m_CHR0Page];
	case 0x1000:	// 
		return CHRRom[(m_VideoAddress&0x0FFF) | m_CHR1Page];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
