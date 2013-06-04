#include <memory.h>

#include "../Emulation/Emulation.h"
#include "VRC2.4.h"

VRC2_4::VRC2_4( const TimingSettings *Timing, int bit0, int bit1, bool halfChr ) : 
	ROM(Timing) 
{
	m_HalfChar = halfChr;
	m_Bit0 = bit0;
	m_Bit1 = bit1;
}

void VRC2_4::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	PrgMask		= (PRGSize - 1) & ~0x1FFF;
	m_CHRMask	= (CHRSize - 1) & ~0x03FF;

	m_NearLastPage = (0xFE << 13) & PrgMask;

	Mirroring = 0;	// Vertical

	m_PRGSwap	= false;
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
	m_IRQMode = false;

	m_Scalar = 341;
}

bool VRC2_4::IRQActive()
{
	return m_IRQActive;
}

void VRC2_4::CPUClock()
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

void VRC2_4::WriteReg( unsigned short addr, unsigned char ch )
{
	// Do some magic to convert VRC2 / VRC4 registers uniformly
	unsigned short reg = (addr & 0xF000) |
		(( addr & m_Bit0 ) ? 2 : 0) |
		(( addr & m_Bit1 ) ? 4 : 0);

	// VRC2a half char
	if( m_HalfChar && reg >= 0xB000 && reg < 0xF000 )
		ch >>= 1;

	switch( reg )
	{
	case 0x9000: case 0x9002:
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
	case 0x9004: case 0x9006:
		m_PRGSwap = (ch & 0x02) != 0;
		break ;
	case 0x8000: case 0x8002: case 0x8004: case 0x8006:
		m_PRGPage0 = (ch << 13) & PrgMask;
		break ;
	case 0xA000: case 0xA002: case 0xA004: case 0xA006:
		m_PRGPage1 = (ch << 13) & PrgMask;
		break ;
	case 0xB000:
		m_CHRPage0 = ((m_CHRPage0 & 0x3C000) | ((ch & 0xF) << 10)) & m_CHRMask;
		break ;
	case 0xB002:
		m_CHRPage0 = ((m_CHRPage0 & 0x03C00) | ((ch & 0xF) << 14)) & m_CHRMask;
		break ;
	case 0xB004:
		m_CHRPage1 = ((m_CHRPage1 & 0x3C000) | ((ch & 0xF) << 10)) & m_CHRMask;
		break ;
	case 0xB006:
		m_CHRPage1 = ((m_CHRPage1 & 0x03C00) | ((ch & 0xF) << 14)) & m_CHRMask;
		break ;
	case 0xC000:
		m_CHRPage2 = ((m_CHRPage2 & 0x3C000) | ((ch & 0xF) << 10)) & m_CHRMask;
		break ;
	case 0xC002:
		m_CHRPage2 = ((m_CHRPage2 & 0x03C00) | ((ch & 0xF) << 14)) & m_CHRMask;
		break ;
	case 0xC004:
		m_CHRPage3 = ((m_CHRPage3 & 0x3C000) | ((ch & 0xF) << 10)) & m_CHRMask;
		break ;
	case 0xC006:
		m_CHRPage3 = ((m_CHRPage3 & 0x03C00) | ((ch & 0xF) << 14)) & m_CHRMask;
		break ;
	case 0xD000:
		m_CHRPage4 = ((m_CHRPage4 & 0x3C000) | ((ch & 0xF) << 10)) & m_CHRMask;
		break ;
	case 0xD002:
		m_CHRPage4 = ((m_CHRPage4 & 0x03C00) | ((ch & 0xF) << 14)) & m_CHRMask;
		break ;
	case 0xD004:
		m_CHRPage5 = ((m_CHRPage5 & 0x3C000) | ((ch & 0xF) << 10)) & m_CHRMask;
		break ;
	case 0xD006:
		m_CHRPage5 = ((m_CHRPage5 & 0x03C00) | ((ch & 0xF) << 14)) & m_CHRMask;
		break ;
	case 0xE000:
		m_CHRPage6 = ((m_CHRPage6 & 0x3C000) | ((ch & 0xF) << 10)) & m_CHRMask;
		break ;
	case 0xE002:
		m_CHRPage6 = ((m_CHRPage6 & 0x03C00) | ((ch & 0xF) << 14)) & m_CHRMask;
		break ;
	case 0xE004:
		m_CHRPage7 = ((m_CHRPage7 & 0x3C000) | ((ch & 0xF) << 10)) & m_CHRMask;
		break ;
	case 0xE006:
		m_CHRPage7 = ((m_CHRPage7 & 0x03C00) | ((ch & 0xF) << 14)) & m_CHRMask;
		break ;
	case 0xF000:
		m_CounterReload = (m_CounterReload & 0xF0) | (ch);
		break ;
	case 0xF002:
		m_CounterReload = (m_CounterReload & 0x0F) | (ch << 4);
		break ;
	case 0xF004:
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
	case 0xF006:
		m_IRQActive = false;
		m_IRQEnable = m_IRQEnaAck;
		break ;
	}
}

unsigned char VRC2_4::Read( unsigned short addr )
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
		return m_BusTrash = PRGRom[(addr & 0x1FFF) | (m_PRGSwap ? m_NearLastPage : m_PRGPage0 )];
	case 0xA000:
		return m_BusTrash = PRGRom[(addr & 0x1FFF) | m_PRGPage1];
	case 0xC000:
		return m_BusTrash = PRGRom[(addr & 0x1FFF) | (m_PRGSwap ? m_PRGPage0 : m_NearLastPage )];
	case 0xE000:
		return m_BusTrash = PRGRom[addr | PrgMask];
	}

	return m_BusTrash;
}

void VRC2_4::Write( unsigned short addr, unsigned char ch )
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

void VRC2_4::VideoAddressLatch( unsigned short Address ) 
{
	m_VideoAddress = Address; 
}

void VRC2_4::VideoWrite( unsigned char Byte )
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

unsigned char VRC2_4::VideoRead() 
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
