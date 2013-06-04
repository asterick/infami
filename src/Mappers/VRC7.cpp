#include <memory.h>

#include "../Emulation/Emulation.h"
#include "VRC7.h"

VRC7::VRC7( const TimingSettings *Timing ) : 
	ROM(Timing) 
{
}

void VRC7::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	PrgMask		= (PRGSize - 1) & ~0x1FFF;
	
	ChrMask		= (CHRSize - 1) & ~0x03FF;

	Mirroring	= 0;	// Vertical

	m_PRGPage0	= 0;
	m_PRGPage1	= 0;

	m_CHRPage0	= 0;
	m_CHRPage1	= 0;
	m_CHRPage2	= 0;
	m_CHRPage3	= 0;
	m_CHRPage4	= 0;
	m_CHRPage5	= 0;
	m_CHRPage6	= 0;
	m_CHRPage7	= 0;

	m_CounterReload = 0;
	m_Counter = 0;

	m_IRQActive = false;
	m_IRQEnaAck = false;
	m_IRQEnable = false;
	m_IRQMode	= false;

	m_Scalar = 341;
}

bool VRC7::IRQActive()
{
	return m_IRQActive;
}

void VRC7::CPUClock()
{
	if( !m_IRQEnable )
		return ;
	
	if( !m_IRQMode )
	{
		m_Scalar -= 3;

		if( m_Scalar <= 0 )
			m_Scalar += 341;
		else
			return;
	}

	if( m_Counter++ == 0xFF )
	{
		m_IRQActive = true;
		m_Counter = m_CounterReload;
	}
}

void VRC7::WriteReg( unsigned short addr, unsigned char ch )
{
	switch( addr )
	{
	case 0x8000:
		m_PRGPage0 = (ch << 13) & PrgMask;
		break ;
	case 0x8008: case 0x8010:
		m_PRGPage1 = (ch << 13) & PrgMask;
		break ;
	case 0x9000:
		m_PRGPage2 = (ch << 13) & PrgMask;
		break ;

	case 0xA000:
		m_CHRPage0 = (ch << 10) & ChrMask;
		break ;
	case 0xA010: case 0xA008:
		m_CHRPage1 = (ch << 10) & ChrMask;
		break ;
	case 0xB000:
		m_CHRPage2 = (ch << 10) & ChrMask;
		break ;
	case 0xB010: case 0xB008:
		m_CHRPage3 = (ch << 10) & ChrMask;
		break ;
	case 0xC000:
		m_CHRPage4 = (ch << 10) & ChrMask;
		break ;
	case 0xC010: case 0xC008:
		m_CHRPage5 = (ch << 10) & ChrMask;
		break ;
	case 0xD000:
		m_CHRPage6 = (ch << 10) & ChrMask;
		break ;
	case 0xD010: case 0xD008: 
		m_CHRPage7 = (ch << 10) & ChrMask;
		break ;

	case 0xE000:
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

	case 0xE008: case 0xE010:
		m_CounterReload = ch;
		break ;
	case 0xF000:
		m_IRQEnaAck = (m_BusTrash & 0x1) != 0;
		m_IRQEnable = (m_BusTrash & 0x2) != 0;
		m_IRQMode = (m_BusTrash & 0x4) != 0;
		if( m_IRQEnable )
		{
			m_Counter = m_CounterReload;
			m_Scalar = 341;
		}
		m_IRQActive = false;
		break ;
	case 0xF008: case 0xF010:
		m_IRQActive = false;
		m_IRQEnable = m_IRQEnaAck;
		break ;

	// VRC7 Audio:
	case 0x9010:	case 0x9030:
		break ;
	}
}

unsigned char VRC7::Read( unsigned short addr )
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
		return m_BusTrash = PRGRom[(addr & 0x1FFF) | m_PRGPage0 ];
	case 0xA000:
		return m_BusTrash = PRGRom[(addr & 0x1FFF) | m_PRGPage1 ];
	case 0xC000:
		return m_BusTrash = PRGRom[(addr & 0x1FFF) | m_PRGPage2 ];
	case 0xE000:
		return m_BusTrash = PRGRom[addr | PrgMask];
	}

	return m_BusTrash;
}

void VRC7::Write( unsigned short addr, unsigned char ch )
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
		WriteReg( addr, ch );
	}
}

void VRC7::VideoAddressLatch( unsigned short Address ) 
{
	m_VideoAddress = Address; 
}

void VRC7::VideoWrite( unsigned char Byte )
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

unsigned char VRC7::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
		switch( m_VideoAddress & 0xC00 )
		{
		case 0x000:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage0 ];
		case 0x400:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage1 ];
		case 0x800:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage2 ];
		case 0xC00:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage3 ];
			break ;
		}
		break ;
	case 0x1000:
		switch( m_VideoAddress & 0xC00 )
		{
		case 0x000:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage4 ];
		case 0x400:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage5 ];
		case 0x800:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage6 ];
		case 0xC00:
			return CHRRom[ (m_VideoAddress & 0x3FF) | m_CHRPage7 ];
			break ;
		}
		break ;
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
