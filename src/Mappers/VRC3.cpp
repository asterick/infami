#include <memory.h>

#include "../Emulation/Emulation.h"
#include "VRC3.h"

void VRC3::Initalize( int Mirroring )
{
	ROM::Initalize( Mirroring );
	m_PRGMask = PrgMask & ~0x3FFF;
	m_PRGPage = 0;

	m_CounterReload = 0;
	m_Counter = 0;

	m_IRQActive = false;
	m_IRQEnaAck = false;
	m_IRQEnable = false;
	m_IRQMode = false;
}

bool VRC3::IRQActive()
{
	return m_IRQActive;
}

void VRC3::CPUClock()
{
	if( !m_IRQEnable )
		return ;

	m_Counter = (m_Counter + 1) & ( m_IRQMode ? 0xFF : 0xFFFF );

	if( m_Counter == 0 )
	{
		m_IRQActive = true;
		m_Counter = m_CounterReload;
	}
}

unsigned char VRC3::Read( unsigned short addr )
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
			return BCKRam[ addr & RamMask ];
		break ;
	case 0x8000:
	case 0xA000:
		return m_BusTrash = PRGRom[(addr & 0x3FFF) | m_PRGPage];
	case 0xC000:
	case 0xE000:
		return m_BusTrash = PRGRom[addr | m_PRGMask];
	}

	return m_BusTrash;
}

void VRC3::Write( unsigned short addr, unsigned char ch )
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
		case 0x8000:	// IRQ Latch 0
			m_CounterReload = (m_CounterReload & 0xFFF0) | ((m_BusTrash&0xF) << 0 );
			break ;
		case 0x9000:	// IRQ Latch 1
			m_CounterReload = (m_CounterReload & 0xFF0F) | ((m_BusTrash&0xF) << 4 );
			break ;
		case 0xA000:	// IRQ Latch 2
			m_CounterReload = (m_CounterReload & 0xF0FF) | ((m_BusTrash&0xF) << 8 );
			break ;
		case 0xB000:	// IRQ Latch 3
			m_CounterReload = (m_CounterReload & 0x0FFF) | ((m_BusTrash&0xF) << 12 );
			break ;
		case 0xC000:	// IRQ Control
			m_IRQEnaAck = (m_BusTrash & 0x1) != 0;
			m_IRQEnable = (m_BusTrash & 0x2) != 0;
			m_IRQMode = (m_BusTrash & 0x4) != 0;
			if( m_IRQEnable )
				m_Counter = m_CounterReload;
			m_IRQActive = false;
			break ;
		case 0xD000:	// IRQ Ack
			m_IRQActive = false;
			m_IRQEnable = m_IRQEnaAck;
			break;
		case 0xF000:	// PRG Select
			m_PRGPage = (m_BusTrash << 14) & m_PRGMask;
			break ;
		}
	}
}

void VRC3::VideoAddressLatch( unsigned short Address )
{
	m_VideoAddress = Address;
}

void VRC3::VideoWrite( unsigned char Byte )
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

unsigned char VRC3::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	// 
		return CHRRam[m_VideoAddress&0x1FFF];
	default:	// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
