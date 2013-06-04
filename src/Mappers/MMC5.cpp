#include <memory.h>

#include "../Emulation/SquareAPU.h"
#include "../Emulation/Emulation.h"
#include "MMC5.h"

MMC5::MMC5( const TimingSettings *Timing ) : 
	ROM(Timing),
	Square1( false ), 
	Square2( false )
{
}

void MMC5::Initalize( int Mirroring )
{	
	ROM::Initalize( Mirroring );

	BckMask = (BCKSize - 1) & ~0x1FFF;
	WrkMask = (WRKSize - 1) & ~0x1FFF;
	PrgMask = (PRGSize - 1) & ~0x1FFF;
	ChrMask = (PRGSize - 1) & ~0x03FF;

	m_ExTile		= 0;

	m_IRQActive = false;
	m_IRQEnable = false;
	m_InFrameFlag = false;
	m_LineCounter = 0;
	m_LineLatch = 0;

	m_EnvelopeCount = 0;

	m_PCMOutput1 = 0;
	m_PCMOutput2 = 0;
	m_PCMSelect  = 0;
	m_AudioAccumulator = 0;
	m_AudioSamples = 0;

	m_NameTableMapping = 0;
	m_CHRExtender	= 0;

	m_PRGMode		= MMC5_PRGMODE_8K;
	m_PRGBank[0]	= 0;
	m_PRGBank[1]	= 0xFF;
	m_PRGBank[2]	= 0xFF;
	m_PRGBank[3]	= 0xFF;
	m_PRGBank[4]	= 0xFF;

	m_CHRMode		= MMC5_CHRMODE_8K;
	m_SPRBank[0]	= 0;
	m_SPRBank[1]	= 0;
	m_SPRBank[2]	= 0;
	m_SPRBank[3]	= 0;
	m_SPRBank[4]	= 0;
	m_SPRBank[5]	= 0;
	m_SPRBank[6]	= 0;
	m_SPRBank[7]	= 0;
	m_CHRBank[0]	= 0;
	m_CHRBank[1]	= 0;
	m_CHRBank[2]	= 0;
	m_CHRBank[3]	= 0;

	for( int i = 0; i < 8; i++ )
		m_FillMode[i]	= false;

	m_EnableVSplit	= false;
	m_RightVSplit	= false;
	m_OffsetVSplit	= 0;
	m_ScrollVSplit	= 0;
	m_BankVSplit	= CHRRom;

	UpdatePRGBanks();
	UpdateCHRBanks();
	UpdateNameBanks();
}

void MMC5::CPUClock()
{
	m_EnvelopeCount += 2;
	if( m_EnvelopeCount >= 14915 )
	{
		if( m_ClockLength )
		{
			Square1.ClockSweep();
			Square2.ClockSweep();
		}
		m_ClockLength = !m_ClockLength;

		Square1.ClockEnvelope();
		Square2.ClockEnvelope();
		m_EnvelopeCount -= 14915;
	}

	m_AudioAccumulator += Square1.ClockOutput() + Square2.ClockOutput();
	m_AudioSamples++;
}

bool MMC5::IRQActive()
{
	return m_IRQActive && m_IRQEnable;
}

unsigned char MMC5::RegisterRead( unsigned short addr )
{
	// ExRAM
	if( addr >= 0x5C00 )
	{
		if( m_ExRAMMode >= MMC5_EXRAM_RAM )
			return m_ExRam[addr & 0x3FF];
		return m_BusTrash;
	}

	switch( addr )
	{
	/* --- AUDIO Registers --- */
	case 0x5010:
		// FUNCTION UNKNOWN
		return 0;
	case 0x5015:
		return
			(Square1.LengthActive() ? 0x01 : 0) |
			(Square2.LengthActive() ? 0x02 : 0);
		break ;

	/* --- IRQ Status --- */
	case 0x5204:	
		{
			unsigned char v =
				(m_IRQActive ? 0x80 : 0x00) |
				(m_InFrameFlag ? 0x40 : 0x00);
		
			m_IRQActive = false;
			return v;
		}

	/* --- 8 bit multiplier --- */
	case 0x5205:
		return m_MultiResult & 0xFF;
	case 0x5206:
		return m_MultiResult >> 8;
	}

	return m_BusTrash;
}

void MMC5::RegisterWrite( unsigned short addr, unsigned char ch )
{
	// ExRAM
	if( addr >= 0x5C00 )
	{
		if( m_ExRAMMode != MMC5_EXRAM_RAM_READONLY )
		{
			m_ExRam[addr & 0x3FF] = ch;
		}
		return ;
	}

	switch( addr )
	{
	/* --- AUDIO Registers --- */
	/* Square 1 */
	case 0x5000:
		Square1.WriteDutyEnv(ch);
		break ;
/*
	case 0x5001:
		Square1.WriteSweep(ch);
		break ;
*/
	case 0x5002:
		Square1.WritePeriodLow(ch);
		break ;
	case 0x5003:
		Square1.WriteLengthPeriod(ch);
		break ;
	/* Square 2 */
	case 0x5004:
		Square2.WriteDutyEnv(ch);
		break ;
/*
	case 0x5005:
		Square2.WriteSweep(ch);
		break ;
*/
	case 0x5006:
		Square2.WritePeriodLow(ch);
		break ;
	case 0x5007:
		Square2.WriteLengthPeriod(ch);
		break ;
	/* PCM Channels */
	case 0x5010:
		m_PCMSelect = ch & 0x01;
		break ;
	case 0x5011:
		if( m_PCMSelect )
			m_PCMOutput2 = ch;
		else
			m_PCMOutput1 = ch;
		break ;
	case 0x5015:
		Square1.WriteEnable( (ch & 0x01) ? true : false );
		Square2.WriteEnable( (ch & 0x02) ? true : false );
		break ;

	/* --- 8 bit multiplier --- */
	case 0x5205:
		m_Multiplicand = ch;
		m_MultiResult = m_Multiplicand * m_Multiplier;
		break ;
	case 0x5206:
		m_Multiplier = ch;
		m_MultiResult = m_Multiplicand * m_Multiplier;
		break ;

	/* --- Bank switch modes --- */
	case 0x5100:
		m_PRGMode = ch & 3;
		UpdatePRGBanks();
		break ;
	case 0x5101:
		m_CHRMode = ch & 3;
		UpdateCHRBanks();
		break ;
	case 0x5104:
		m_ExRAMMode = ch & 3;
		UpdateNameBanks();
		break ;

	/* --- WRK Ram protect --- */
	case 0x5102:
		m_WRKProtect1 = (ch & 3) == 2;
		break ;
	case 0x5103:
		m_WRKProtect2 = (ch & 3) == 1;
		break ;

	/* --- Nametable mapping --- */
	case 0x5105: 
		m_NameTableMapping = ch;
		UpdateNameBanks();
		break ;
	case 0x5106:
		m_FillModeTile = ch;
		break ;
	case 0x5107:
		m_FillModeAttr = (ch & 3) * 0x55;
		break ;

	/* --- CPU Bank registers --- */
	case 0x5113:
		m_PRGBank[0] = ch & 0x7F;	// FORCE RAM
		UpdatePRGBanks();
		break ;
	case 0x5114:
	case 0x5115:
	case 0x5116:
		m_PRGBank[addr-0x5113] = ch;
		UpdatePRGBanks();
		break ;
	case 0x5117:
		m_PRGBank[4] = ch | 0x80;
		UpdatePRGBanks();
		break ;

	/* --- PPU Bank registers --- */
	case 0x5120:
	case 0x5121:
	case 0x5122:
	case 0x5123:
	case 0x5124:
	case 0x5125:
	case 0x5126:
	case 0x5127:
		m_SPRBank[addr-0x5120] = ch;
		UpdateCHRBanks();
		break ;
	case 0x5128:
	case 0x5129:
	case 0x512A:
	case 0x512B:
		m_CHRBank[addr-0x5128] = ch;
		UpdateCHRBanks();
		break ;
	case 0x5130:
		m_CHRExtender = ch;
		UpdateCHRBanks();
		break ;

	case 0x5200:	// VSPLIT MODE
		m_EnableVSplit	= (ch & 0x80) != 0;
		m_RightVSplit	= (ch & 0x40) != 0;
		m_OffsetVSplit	= (ch & 0x1F) << 2;	// In VRAM Loads
		break ;
	case 0x5201:	// VSPLIT SCROLL
		m_ScrollVSplit	= ch;
		break ;
	case 0x5202:	// VSPLIT BANK
		m_BankVSplit	= ((ch << 12) & ChrMask) + CHRRom;
		break ;

	case 0x5203:	// IRQ COUNTER
		m_LineLatch = ch;
		break ;
	case 0x5204:	// IRQ STATUS
		m_IRQEnable = (ch & 0x80) != 0;
		m_IRQActive = false;
		break ;	
	}
}

void MMC5::PrgSetPage( int page, int bank, int mask )
{
	if( m_PRGBank[bank] & 0x80 )
	{
		m_PRGPage[page] = PRGRom + (((m_PRGBank[bank] & mask) << 13) & PrgMask);
		m_PRGWrite[page] = false;
	}	
	else
	{
		unsigned char*	WrkRam	= (m_PRGBank[bank] & 0x04) ? WRKRam : BCKRam;
		int				Mask	= (m_PRGBank[bank] & 0x04) ? WrkMask : BckMask;

		if( WrkRam )
		{
			m_PRGPage[page] = WrkRam + (((m_PRGBank[bank] & mask) << 13) & Mask);
			m_PRGWrite[page] = true;		
		}
		else
		{
			m_PRGPage[page] = NULL;
			m_PRGWrite[page] = false;
		}		
	}
}

void MMC5::PrgCopyPage( int page_dst, int page_src )
{
	if( m_PRGPage[page_src] )
	{
		m_PRGPage[page_dst] = m_PRGPage[page_src] + 0x2000;
		m_PRGWrite[page_dst] = m_PRGWrite[page_src];
	}
	else
	{
		m_PRGPage[page_dst] = NULL;
		m_PRGWrite[page_dst] = false;
	}
}

void MMC5::UpdatePRGBanks()
{
	PrgSetPage( 0, 0, 0x7F );

	switch( m_PRGMode )
	{
	case MMC5_PRGMODE_32K:
		PrgSetPage( 1, 4, 0x7C );
		PrgCopyPage( 2, 1 );
		PrgCopyPage( 3, 2 );
		PrgCopyPage( 4, 3 );
		break ;
	case MMC5_PRGMODE_16K:
		PrgSetPage( 1, 2, 0x7E );
		PrgCopyPage( 2, 1 );
		PrgSetPage( 3, 4, 0x7E );	
		PrgCopyPage( 4, 3 );
		break ;
	case MMC5_PRGMODE_MIXED:
		PrgSetPage( 1, 2, 0x7E );
		PrgCopyPage( 2, 1 );
		PrgSetPage( 3, 3, 0x7F );
		PrgSetPage( 4, 4, 0x7F );
		break ;
	case MMC5_PRGMODE_8K:
		PrgSetPage( 1, 1, 0x7F );
		PrgSetPage( 2, 2, 0x7F );
		PrgSetPage( 3, 3, 0x7F );
		PrgSetPage( 4, 4, 0x7F );

		break ;
	}
}

void MMC5::UpdateCHRBanks()
{
	switch( m_CHRMode )
	{
	case MMC5_CHRMODE_8K:
		m_SPRPage[0] = (m_SPRBank[7] << 13 & ChrMask) + CHRRom;
		m_CHRPage[0] = (m_CHRBank[3] << 13 & ChrMask) + CHRRom;
		
		for( int i = 0; i < 7; i++ )
		{
			m_SPRPage[i+1] = m_SPRPage[ i ] + 0x400;
			m_CHRPage[i+1] = m_CHRPage[ i ] + 0x400;
		}

		break ;
	case MMC5_CHRMODE_4K:
		m_SPRPage[0] = (m_SPRBank[3] << 12 & ChrMask) + CHRRom;
		m_SPRPage[4] = (m_SPRBank[7] << 12 & ChrMask) + CHRRom;
		m_CHRPage[0] = (m_CHRBank[3] << 12 & ChrMask) + CHRRom;
		m_CHRPage[4] = (m_CHRBank[3] << 12 & ChrMask) + CHRRom;

		for( int i = 0; i < 3; i++ )
		{
			m_SPRPage[i+1] = m_SPRPage[ i ] + 0x400;
			m_SPRPage[i+5] = m_SPRPage[i+4] + 0x400;
			m_CHRPage[i+1] = m_CHRPage[ i ] + 0x400;
			m_CHRPage[i+5] = m_CHRPage[i+4] + 0x400;
		}
		break ;
	case MMC5_CHRMODE_2K:
		for( int i = 0; i < 8; i+=2 )
		{
			m_SPRPage[ i ] = (((m_SPRBank[ i +1] << 11) | (m_CHRExtender << 19)) & ChrMask) + CHRRom;
			m_SPRPage[i+1] = m_SPRPage[ i ] + 0x400;
			m_CHRPage[ i ] = (((m_CHRBank[i&3+1] << 11) | (m_CHRExtender << 19)) & ChrMask) + CHRRom;
			m_CHRPage[i+1] = m_CHRPage[ i ] + 0x400;
		}
		break ;
	case MMC5_CHRMODE_1K:
		for( int i = 0; i < 8; i++ )
		{
			m_SPRPage[i] = (((m_SPRBank[ i ] << 10) | (m_CHRExtender << 18)) & ChrMask) + CHRRom;
			m_CHRPage[i] = (((m_CHRBank[i&3] << 10) | (m_CHRExtender << 18)) & ChrMask) + CHRRom;
		}
		break ;
	}
}

void MMC5::UpdateNameBanks()
{
	for( int i = 0; i < 4; i++ )
	{
		unsigned char* table;
		bool fill;

		switch( (m_NameTableMapping >> (i << 1)) & 3 )
		{
		case 0:
			fill = false;
			table = m_NameTable0;
			break ;
		case 1:
			fill = false;
			table = m_NameTable1;
			break ;
		case 2:
			fill = false;

			if( m_ExRAMMode == MMC5_EXRAM_NAMETABLE ||
				m_ExRAMMode == MMC5_EXRAM_ATTRCHREXT )
				table = m_ExRam;
			else
				table = NULL;
			break ;
		default:
			fill = true;
			table = NULL ;
			return ;
		}

		m_FillMode[8+i] = m_FillMode[12+i] = fill;
		m_SPRPage[i+ 8] =
		m_SPRPage[i+12] =
		m_CHRPage[i+ 8] =
		m_CHRPage[i+12] = table ;
	}
}

unsigned char MMC5::Read( unsigned short addr )	
{
	switch( addr & 0xE000 )
	{
	case 0x0000:
		return m_BusTrash = SystemRam[addr&0x7FF];
	case 0x2000:
		return m_BusTrash = PPURead( addr, m_BusTrash );
	case 0x4000:
		if( addr < 0x5000 )
			return m_BusTrash = APURead( addr, m_BusTrash );
		else
			return m_BusTrash = RegisterRead( addr );
	case 0x6000:
		if( m_PRGPage[0] )
			return m_BusTrash = m_PRGPage[0][addr & 0x1FFF];
		break ;
	case 0x8000:
		if( m_PRGPage[1] )
			return m_BusTrash = m_PRGPage[1][addr & 0x1FFF];
		break ;
	case 0xA000:
		if( m_PRGPage[2] )
			return m_BusTrash = m_PRGPage[2][addr & 0x1FFF];
		break ;
	case 0xC000:
		if( m_PRGPage[3] )
			return m_BusTrash = m_PRGPage[3][addr & 0x1FFF];
		break ;
	case 0xE000:
		return m_BusTrash = m_PRGPage[4][addr & 0x1FFF];
	}

	return m_BusTrash;
}

void MMC5::Write( unsigned short addr, unsigned char ch )
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
		if( addr < 0x5000 )
			APUWrite( addr, m_BusTrash );
		else
			RegisterWrite( addr, m_BusTrash );
		break ;
	case 0x6000:
		if( m_WRKProtect1 && m_WRKProtect2 && m_PRGWrite[0] )
			m_PRGPage[0][addr & 0x1FFF] = ch;
		break ;
	case 0x8000:
		if( m_WRKProtect1 && m_WRKProtect2 && m_PRGWrite[1] )
			m_PRGPage[1][addr & 0x1FFF] = ch;
		break ;
	case 0xA000:
		if( m_WRKProtect1 && m_WRKProtect2 && m_PRGWrite[2] )
			m_PRGPage[2][addr & 0x1FFF] = ch;
		break ;
	case 0xC000:
		if( m_WRKProtect1 && m_WRKProtect2 && m_PRGWrite[3] )
			m_PRGPage[3][addr & 0x1FFF] = ch;
		break ;
	}
}

float MMC5::StreamAudio()
{
	float output = ROM::StreamAudio();

	output += m_AudioAccumulator / m_AudioSamples / 64.0f;

	m_AudioSamples = 0;
	m_AudioAccumulator = 0;

	return output;
}

void MMC5::VideoAddressLatch( unsigned short Address ) 
{
	if( m_ReadTimeout )
	{
		m_HorizontalCounter = 0;
		m_InFrameFlag = false;
		m_IRQActive = false;
	}
	m_ReadTimeout = true;

	m_VideoAddress = Address;
}

void MMC5::VideoWrite( unsigned char Byte )
{
	if( m_VideoAddress & 0x2000 )
	{
		int page = (m_VideoAddress >> 10) & 0xF;
		unsigned char* bank = m_SPRPage[page];
		
		if( bank )
			bank[m_VideoAddress&0x3FF] = Byte;
	}
}

unsigned char MMC5::VideoRead() 
{ 
	m_ReadTimeout = false;
	
	if( m_VideoAddress & 0x2000 )
	{
		if( ++m_SyncPulses == 3 )
		{
			if( m_InFrameFlag )
			{
				if( m_LineCounter == m_LineLatch )
					m_IRQActive = true;
				m_LineCounter++;
			}
			else
			{
				m_LineCounter = 0;
				m_InFrameFlag = true;
			}

			m_HorizontalCounter = 0;
		}
		else
		{
			m_HorizontalCounter++;
		}
	}
	else
	{
		m_SyncPulses = 0;
		m_HorizontalCounter++;
	}

	if( m_HorizontalCounter == 169 )
	{
	}

	if( m_HorizontalCounter == 128 )
		m_InSprite = true;
	else if( m_HorizontalCounter == 160 )
		m_InSprite = false;

	// MMC5 Rendering overrides
	int page = (m_VideoAddress >> 10) & 0xF;
	unsigned char* bank = m_InSprite ? m_SPRPage[page] : m_CHRPage[page];

	if( m_InSprite || m_ExRAMMode != MMC5_EXRAM_ATTRCHREXT )
	{
		if( m_FillMode[page] )
		{
			if( (m_VideoAddress & 0x3C0) == 0x3C0 )
				return m_FillModeAttr;
			else
				return m_FillModeTile;
		}
		else if( bank )
			return bank[m_VideoAddress&0x3FF];
		else
			return m_HorizontalCounter;
	}
	else
	{
		if( m_VideoAddress & 0x2000 )
		{
			if( (m_VideoAddress & 0x3C0) == 0x3C0 )
			{
				return m_ExAttribute * 0x55;
			}
			else
			{
				m_ExAttribute	= m_ExRam[ m_VideoAddress & 0x3FF ];
				m_ExTile		= ((m_ExAttribute & 0x3F) << 12) | (m_CHRExtender << 18) & ChrMask;
				m_ExAttribute >>= 6;
				
				if( m_FillMode[page] )
				{
					if( (m_VideoAddress & 0x3C0) == 0x3C0 )
						return m_FillModeAttr;
					else
						return m_FillModeTile;
				}
				else if( bank )
					return bank[m_VideoAddress&0x3FF];
				else
					return 0;
			}
		}
		else
		{
			return CHRRom[ (m_VideoAddress & 0xFFF) | m_ExTile ];
		}
	}
}
