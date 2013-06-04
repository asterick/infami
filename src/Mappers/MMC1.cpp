#include "../Emulation/Emulation.h"

#include "NROM.h"
#include "MMC1.h"

void MMC1::Initalize( int Mirroring )
{
	ROM::Initalize( Mirroring );

	m_CHRMask = (CHRSize - 1) & ~0xFFF;
	m_PRGMask = PrgMask & ~0x3FFF;

	SetMirrorZero();

	m_PrgBank = true;
	m_16kPRG = true;
	m_4kCHR  = true;
	m_WriteProtect = false;

	m_PrgPage = 0;
	m_ChrPage0 = 0;
	m_ChrPage1 = 0;

	SetBanks();
}

void MMC1::SetBanks()
{
	if( m_16kPRG )
	{
		if( !m_PrgBank )
		{
			m_PRGPageLo = ((     0x0F << 14) | ((m_ChrPage0 & 0x10) << 14)) & m_PRGMask;
			m_PRGPageHi = ((m_PrgPage << 14) | ((m_ChrPage0 & 0x10) << 14)) & m_PRGMask;
		}
		else
		{
			m_PRGPageLo = ((m_PrgPage << 14) | ((m_ChrPage0 & 0x10) << 14)) & m_PRGMask;
			m_PRGPageHi = ((     0x0F << 14) | ((m_ChrPage0 & 0x10) << 14)) & m_PRGMask;
		}
	}
	else
	{
		m_PRGPageLo = (((m_PrgPage & 0xFE) << 14) | ((m_ChrPage0 & 0x10) << 15)) & m_PRGMask;
		m_PRGPageHi = (((m_PrgPage | 0x01) << 14) | ((m_ChrPage0 & 0x10) << 15)) & m_PRGMask;
	}

	if( m_4kCHR )
	{
		m_CHRPageLo = (m_ChrPage0 << 12) & m_CHRMask;
		m_CHRPageHi = (m_ChrPage1 << 12) & m_CHRMask;
	}
	else
	{
		m_CHRPageLo = ((m_ChrPage0 & 0xFE) << 12) & m_CHRMask;
		m_CHRPageHi = ((m_ChrPage0 | 0x01) << 12) & m_CHRMask;
	}
}

void MMC1::WriteShifting( int bank, unsigned char ch )
{
	if( m_Ignore )
		return ;

	m_Ignore = true;

	if( ch & 0x80 )
	{
		ShiftReg = 0;
		Bits = 0;

		return ;
	}

	ShiftReg |= (ch & 1) << (Bits++);

	if( Bits == 5 )
	{
		switch( bank & 0xE000 )
		{
		case 0x8000:
			switch( ShiftReg & 0x3 )
			{
			case 0:
				SetMirrorZero();
				break;
			case 1:
				SetMirrorOne();
				break;
			case 2:
				SetMirrorVertical();
				break ;
			case 3:
				SetMirrorHorizontal();
				break ;
			}
			m_PrgBank = (ShiftReg & 0x04) != 0;
			m_16kPRG = (ShiftReg & 0x08) != 0;
			m_4kCHR = (ShiftReg & 0x10) != 0;
			break ;
		case 0xA000:
			m_ChrPage0 = ShiftReg;
			break ;
		case 0xC000:
			m_ChrPage1 = ShiftReg;
			break ;
		case 0xE000:
			m_PrgPage = ShiftReg & 0xF;
			m_WriteProtect = (ShiftReg & 0x10) != 0;
			break ;
		}

		SetBanks();

		ShiftReg = 0;
		Bits = 0;
	}
}

unsigned char MMC1::Read( unsigned short addr )
{
	m_Ignore = false;

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
		break;
	case 0x8000:
	case 0xA000:
		return m_BusTrash = PRGRom[(addr & 0x3FFF) | m_PRGPageLo];
	case 0xC000:
	case 0xE000:
		return m_BusTrash = PRGRom[(addr & 0x3FFF) | m_PRGPageHi];
	}

	return m_BusTrash;
}

void MMC1::Write( unsigned short addr, unsigned char ch )
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
		if( BCKRam && !m_WriteProtect )
			BCKRam[ addr & RamMask ] = m_BusTrash;
		break;
	case 0x8000:
	case 0xA000:
	case 0xC000:
	case 0xE000:
		WriteShifting( addr & 0xE000, ch );
		break ;
	}
}

void MMC1::VideoAddressLatch( unsigned short Address )
{
	m_VideoAddress = Address;
}

void MMC1::VideoWrite( unsigned char Byte )
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

unsigned char MMC1::VideoRead() 
{ 
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
		return CHRRom[(m_VideoAddress&0x0FFF)|m_CHRPageLo];
	case 0x1000:	// 
		return CHRRom[(m_VideoAddress&0x0FFF)|m_CHRPageHi];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0; 
}
