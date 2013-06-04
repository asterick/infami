#include "../Emulation/Emulation.h"
#include "NROM.h"
#include "MMC3.h"

void MMC3::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	m_PRGMask = PrgMask & ~0x1FFF;
	m_CHRMask = (CHRSize - 1) & ~0x03FF;

	m_IRQEnable = false;
	m_IRQActive = false;

	m_IRQReload = 0;
	m_IRQCounter = 0;
	m_EdgeTimeout = 0;

	m_BankSelect = 0;
	m_PRGLockLo  = false;
	m_CHRSwapLo  = false;

	for( int i = 0; i < 8 ; i++ )
		m_Banks[i] = 0;

	Mirroring = 0;

	UpdateBanks();
}

bool MMC3::IRQActive()
{
	return m_IRQEnable && m_IRQActive;
}

void MMC3::ClockIRQ( int edge )
{
	if( !edge )
	{
		if( !(--m_EdgeTimeout) )
			m_A12Edge = false;

		return ;
	}
	else if( edge )
	{
		m_EdgeTimeout = 16;

		if( m_A12Edge )
			return ;

		m_A12Edge = true;
	}


	if( m_ReloadIRQ || m_IRQCounter == 0 )
	{
		m_ReloadIRQ = false;
		m_IRQCounter = m_IRQReload;
	}
	else
	{
		m_IRQCounter--;
	}

	if( m_IRQCounter == 0 && m_IRQEnable )
		m_IRQActive = true;
}

void MMC3::UpdateBanks()
{
	if( m_PRGLockLo )
	{
		m_PRGBank0 = (0xFE << 13) & m_PRGMask;
		m_PRGBank1 = (m_Banks[7] << 13) & m_PRGMask;
		m_PRGBank2 = (m_Banks[6] << 13) & m_PRGMask;
	}
	else
	{
		m_PRGBank0 = (m_Banks[6] << 13) & m_PRGMask;
		m_PRGBank1 = (m_Banks[7] << 13) & m_PRGMask;
		m_PRGBank2 = (0xFE << 13) & m_PRGMask;
	}

	if( m_CHRSwapLo )
	{
		m_CHRBank0 = (m_Banks[2] << 10) & m_CHRMask;
		m_CHRBank1 = (m_Banks[3] << 10) & m_CHRMask;
		m_CHRBank2 = (m_Banks[4] << 10) & m_CHRMask;
		m_CHRBank3 = (m_Banks[5] << 10) & m_CHRMask;
		m_CHRBank4 = ((m_Banks[0]&0xFE) << 10) & m_CHRMask;
		m_CHRBank5 = m_CHRBank4 + 0x400;
		m_CHRBank6 = ((m_Banks[1]&0xFE) << 10) & m_CHRMask;
		m_CHRBank7 = m_CHRBank6 + 0x400;
	}
	else
	{
		m_CHRBank0 = ((m_Banks[0]&0xFE) << 10) & m_CHRMask;
		m_CHRBank1 = m_CHRBank0 + 0x400;
		m_CHRBank2 = ((m_Banks[1]&0xFE) << 10) & m_CHRMask;
		m_CHRBank3 = m_CHRBank2 + 0x400;
		m_CHRBank4 = (m_Banks[2] << 10) & m_CHRMask;
		m_CHRBank5 = (m_Banks[3] << 10) & m_CHRMask;
		m_CHRBank6 = (m_Banks[4] << 10) & m_CHRMask;
		m_CHRBank7 = (m_Banks[5] << 10) & m_CHRMask;
	}
}

unsigned char MMC3::Read( unsigned short addr )
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
		return m_BusTrash = PRGRom[(addr&0x1FFF) | m_PRGMask];
	}

	return m_BusTrash;
}

void MMC3::Write( unsigned short addr, unsigned char ch )
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
		if( addr & 1 )	// Bank Data
		{
			m_Banks[m_BankSelect] = m_BusTrash;
		}
		else			// Bank Select
		{
			m_BankSelect = m_BusTrash & 7;
			m_PRGLockLo  = (m_BusTrash & 0x40) != 0;
			m_CHRSwapLo  = (m_BusTrash & 0x80) != 0;
		}
		UpdateBanks();
		break ;
	case 0xA000:
		if( addr & 1 )	// PRG Ram Protect
			;	// Ignoring Ram Protect for now
		else			// Mirroring
		{
			if( ch & 1 )
				SetMirrorHorizontal();
			else
				SetMirrorVertical();
		}
		break ;
	case 0xC000:
		if( addr & 1 )	// IRQ Reload
			m_ReloadIRQ = true;
		else			// IRQ Latch
			m_IRQReload = m_BusTrash;
		break ;
	case 0xE000:
		if( addr & 1 )	// IRQ Enable
			m_IRQEnable = true;
		else			// IRQ Disable
			m_IRQActive = m_IRQEnable = false;
		break ;
	}
}

void MMC3::VideoAddressLatch( unsigned short Address )
{
	ClockIRQ(Address & 0x1000);
	m_VideoAddress = Address;
}

void MMC3::VideoWrite( unsigned char Byte )
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

unsigned char MMC3::VideoRead() 
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
		return GetNameTable(m_VideoAddress);
	}
	return 0; 
}


