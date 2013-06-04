#include <memory.h>

#include "../Emulation/Emulation.h"
#include "Sunsoft3.h"

void Sunsoft3::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	m_PRGMask = PrgMask & ~0x3FFF;
	m_CHRMask = (CHRSize - 1) & ~0x07FF;

	m_PRGBank	= 0;
	m_CHRBank0	= 0;
	m_CHRBank1	= 0;
	m_CHRBank2	= 0;
	m_CHRBank3	= 0;
	m_IRQActive = false;
	m_IRQEnable = false;
	m_WriteLo	= false;

	Mirroring = 0;
	m_Counter = 0;
}

bool Sunsoft3::IRQActive()
{
	return m_IRQActive;
}

void Sunsoft3::CPUClock()
{
	if( !m_IRQEnable )
		return ;

	if( m_Counter-- == 0 )
	{
		m_IRQActive = true;
		m_IRQEnable = false;
	}
}

unsigned char Sunsoft3::Read( unsigned short addr )
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

void Sunsoft3::Write( unsigned short addr, unsigned char ch )
{
	m_BusTrash = ch;
	switch( addr & 0xF000 )
	{
	case 0x0000:
	case 0x1000:
		SystemRam[addr&0x7FF] = ch;
		break ;
	case 0x2000:
	case 0x3000:
		PPUWrite( addr, m_BusTrash );
		break ;
	case 0x4000:
	case 0x5000:
		APUWrite( addr, m_BusTrash );
		break ;
	case 0x6000:
	case 0x7000:
		if( BCKRam )
			BCKRam[ addr & RamMask ] = ch;
		break ;
	case 0x8000:
		if( ~addr & 0x0800 )
			return ;

		m_CHRBank0 = (m_BusTrash << 11) & m_CHRMask;
		break;
	case 0x9000:
		if( ~addr & 0x0800 )
			return ;

		m_CHRBank1 = (m_BusTrash << 11) & m_CHRMask;
		break;
	case 0xA000:
		if( ~addr & 0x0800 )
			return ;

		m_CHRBank2 = (m_BusTrash << 11) & m_CHRMask;
		break ;
	case 0xB000:		
		if( ~addr & 0x0800 )
			return ;

		m_CHRBank3 = (m_BusTrash << 11) & m_CHRMask;
		break;
	case 0xC000:
		if( ~addr & 0x0800 )
			return ;

		if( m_WriteLo )				
			m_Counter = (m_Counter & 0xFF00) | m_BusTrash;
		else
			m_Counter = (m_Counter & 0x00FF) | (m_BusTrash << 8);
			
		m_WriteLo = !m_WriteLo;

		break ;
	case 0xD000:
		if( ~addr & 0x0800 )
			return ;

		m_IRQEnable = (m_BusTrash & 0x10) != 0;
		m_WriteLo = false;
		m_IRQActive = false;
		break;
	case 0xE000:
		if( ~addr & 0x0800 )
			return ;

		switch( ch & 0x3 )
		{
		case 0:
			SetMirrorVertical();
			break ;
		case 1:
			SetMirrorHorizontal();
			break ;
		case 2:
			SetMirrorZero();
			break ;
		case 3:
			SetMirrorOne();
			break ;
		}
		break ;
	case 0xF000:
		if( ~addr & 0x0800 )
			return ;

		m_PRGBank = (m_BusTrash << 14) & m_PRGMask;
		break;
	}
}

void Sunsoft3::VideoAddressLatch( unsigned short Address )
{
	m_VideoAddress = Address;
}

void Sunsoft3::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		CHRRam[m_VideoAddress&0x1FFF] = Byte;
		break ;
	default:	// Name tables
		SetNameTable( m_VideoAddress, Byte );
		break ;
	}
}

unsigned char Sunsoft3::VideoRead() 
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
