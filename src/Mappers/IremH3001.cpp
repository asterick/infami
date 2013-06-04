#include "../Emulation/Emulation.h"
#include "NROM.h"
#include "IremH3001.h"

void IremH3001::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	m_PRGMask = PrgMask & ~0x1FFF;
	m_CHRMask = (CHRSize - 1) & ~0x03FF;

	m_IRQEnable = false;
	m_IRQActive = false;

	m_IRQReload = 0;
	m_IRQCounter = 0;

	m_PRGLockLo  = false;
	m_CHRSwapLo  = false;

	m_CHRBank0 = 0;
	m_CHRBank1 = 0;
	m_CHRBank2 = 0;
	m_CHRBank3 = 0;
	m_CHRBank4 = 0;
	m_CHRBank5 = 0;
	m_CHRBank6 = 0;
	m_CHRBank7 = 0;

	m_PRGBank0 = (0x00 << 13) & m_PRGMask;
	m_PRGBank1 = (0x01 << 13) & m_PRGMask;
	m_PRGBank2 = (0xFE << 13) & m_PRGMask;
}

bool IremH3001::IRQActive()
{
	return m_IRQActive;
}

void IremH3001::CPUClock()
{
	if( !m_IRQEnable )
		return ;

	if( --m_IRQCounter == 0 )
	{
		m_IRQActive = true;
		m_IRQEnable = false;
	}
}

unsigned char IremH3001::Read( unsigned short addr )
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
		return m_BusTrash = PRGRom[(addr&0x1FFF) | m_PRGBank0];
	case 0xA000:
		return m_BusTrash = PRGRom[(addr&0x1FFF) | m_PRGBank1];
	case 0xC000:
		return m_BusTrash = PRGRom[(addr&0x1FFF) | m_PRGBank2];
	case 0xE000:
		return m_BusTrash = PRGRom[addr | m_PRGMask];
	}

	return m_BusTrash;
}

void IremH3001::Write( unsigned short addr, unsigned char ch )
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
			BCKRam[ addr & RamMask ] = m_BusTrash;
	default:
		switch( addr )
		{
		case 0x9000:
			if( m_BusTrash & 0x40 )
				SetMirrorHorizontal();
			else
				SetMirrorVertical();
			break ;
		case 0x9003:
			m_IRQActive = false;
			m_IRQEnable = (m_BusTrash & 0x80) ? true : false;
			break ;
		case 0x9004:
			m_IRQCounter = m_IRQReload;
			m_IRQActive = false;
			break ;
		case 0x9005:
			m_IRQReload = (m_IRQReload & 0x00FF) | (m_BusTrash << 8);
			break ;
		case 0x9006:
			m_IRQReload = (m_IRQReload & 0xFF00) | m_BusTrash;
			break ;

		// CHR-ROM Banks
		case 0xB000:
			m_CHRBank0 = (m_BusTrash << 10) & m_CHRMask;
			break ;
		case 0xB001:
			m_CHRBank1 = (m_BusTrash << 10) & m_CHRMask;
			break ;
		case 0xB002:
			m_CHRBank2 = (m_BusTrash << 10) & m_CHRMask;
			break ;
		case 0xB003:
			m_CHRBank3 = (m_BusTrash << 10) & m_CHRMask;
			break ;
		case 0xB004:
			m_CHRBank4 = (m_BusTrash << 10) & m_CHRMask;
			break ;
		case 0xB005:
			m_CHRBank5 = (m_BusTrash << 10) & m_CHRMask;
			break ;
		case 0xB006:
			m_CHRBank6 = (m_BusTrash << 10) & m_CHRMask;
			break ;
		case 0xB007:
			m_CHRBank7 = (m_BusTrash << 10) & m_CHRMask;
			break ;

		// PRG-ROM Banks
		case 0x8000:
			m_PRGBank0 = (m_BusTrash << 13) & m_PRGMask;
			break ;
		case 0xA000:
			m_PRGBank1 = (m_BusTrash << 13) & m_PRGMask;
			break ;
		case 0xC000:
			m_PRGBank2 = (m_BusTrash << 13) & m_PRGMask;
			break ;
		}
		break ;
	}
}

void IremH3001::VideoAddressLatch( unsigned short Address )
{
	m_VideoAddress = Address;
}

void IremH3001::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		CHRRam[ m_VideoAddress & 0x1FFF ] = Byte;
		break ;
	default:		// Name tables
		SetNameTable( m_VideoAddress, Byte );
		break ;
	}
}

unsigned char IremH3001::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
		switch( m_VideoAddress & 0xC00 )
		{
		case 0x000:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRBank0 ];
		case 0x400:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRBank1 ];
		case 0x800:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRBank2 ];
		case 0xC00:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRBank3 ];
			break ;
		}
		break ;
	case 0x1000:
		switch( m_VideoAddress & 0xC00 )
		{
		case 0x000:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRBank4 ];
		case 0x400:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRBank5 ];
		case 0x800:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRBank6 ];
		case 0xC00:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRBank7 ];
			break ;
		}
		break ;
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}


