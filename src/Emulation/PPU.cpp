#include "Emulation.h"

PPU::PPU( int verticalLines )
{
	m_VerticalBlankPeriod = verticalLines * 341;
	
	m_HPos	= 0;						// Start at pixel clock 0
	m_VPos	= 0;						// Start at active display, first active pixel
	m_Pixel	= m_Frame;
	
	m_VBlank = 0;
	m_Sprite0Hit = 0;
	m_SpriteOverflow = 0;
	m_Intensify = 0;
	m_GenerateNMI = false;
	m_FrameToggle = false;
	m_VRAMAddress = 0;
	m_ScrollHolding = 0;

	m_PFNoClip	= false;
	m_OAMNoClip	= false;
	m_PFEnable	= false;
	m_OAMEnable = false;

	// Prevent buffer overflow
	m_PFPattern = 0;
	m_OBJPattern = 0;
	m_OAMAddress = 0;
	
	m_NMISupressable = 0;

	m_SpritesFound = 0;

	m_ScrollHolding = 0;
	m_ScrollHorizontal = 0;

	m_NeedVideoRead = false;			// FIFO is empty

	for( int i = 0; i < PPU_FRAME; i++ )
		m_Frame[i] = 0;

	for( int i = 0; i < 256; i++ )
		m_OAMMemory[i] = m_PFSerializer[i] = m_OBJSerializer[i] = 0;

	for( int i = 0; i < 32; i++ )
		m_PaletteConvertBus[i] = ( (i & 0x10) && !(i&3) ) ? (i&0xF) : i;

	for( int i = 0; i < 0x400; i++ )
	{
		// --- Input
		// 0x00F = Playfield color
		// 0x0F0 = Sprite color
		// 0x100 = Zero flag
		// 0x200 = Priority encoder

		// --- OUTPUT
		// 0x1F  decoded color
		// 0x40  zero flag

		unsigned char color = 0;

		// Priority encoder
		if( (i & 0x200) )	// PF priority
			color = (i & 0x003) ? (i & 0xF) : (((i >> 4) & 0xF) | 0x10);
		else				// SPR Priority
			color = (i & 0x030) ? (((i >> 4) & 0xF) | 0x10) : (i & 0xF);

		// color 0 for any pallete points to color 0 pallete 0
		color = (color & 3) ? (color & 0x1F) : 0;

		// Zero hit flag
		if( (i & 0x100) && ( i & 0x30 ) && ( i & 0x03 ) )
			color |= 0x40;

		m_PriorityEncoder[i] = color;
	}

	for( int i = 0; i < 0x20; i++ )
	{
		m_Palette[i] = i;
	}
}

unsigned char PPU::GetNameTable( unsigned short Address )
{
	int table = (Address & 0xC00) >> 10;

	return m_NameTables[table][Address & 0x3FF];
}

void PPU::SetNameTable( unsigned short Address, unsigned char Byte )
{
	int table = (Address & 0xC00) >> 10;

	m_NameTables[table][Address & 0x3FF] = Byte;
}

void PPU::SetMirrorPage( int page, unsigned char *data )
{
	m_NameTables[page] = data;
}

void PPU::SetMirrorVertical()
{
	m_NameTables[0] = m_NameTable0;
	m_NameTables[1] = m_NameTable1;
	m_NameTables[2] = m_NameTable0;
	m_NameTables[3] = m_NameTable1;
}

void PPU::SetMirrorHorizontal()
{
	m_NameTables[0] = m_NameTable0;
	m_NameTables[1] = m_NameTable0;
	m_NameTables[2] = m_NameTable1;
	m_NameTables[3] = m_NameTable1;
}

void PPU::SetMirrorFourScreen()
{
	m_NameTables[0] = m_NameTable0;
	m_NameTables[1] = m_NameTable1;
	m_NameTables[2] = m_NameTable2;
	m_NameTables[3] = m_NameTable3;
}

void PPU::SetMirrorZero()
{
	m_NameTables[0] = m_NameTable0;
	m_NameTables[1] = m_NameTable0;
	m_NameTables[2] = m_NameTable0;
	m_NameTables[3] = m_NameTable0;
}

void PPU::SetMirrorOne()
{
	m_NameTables[0] = m_NameTable1;
	m_NameTables[1] = m_NameTable1;
	m_NameTables[2] = m_NameTable1;
	m_NameTables[3] = m_NameTable1;
}

inline void PPU::WriteHorizontal( unsigned char Byte )
{
	m_ScrollHolding    = (m_ScrollHolding & 0x7FE0) | (Byte >> 3);
	m_ScrollHorizontal = Byte & 0x7;	// Only the lower 3 bits are saved (8 to 1 x 2 mux)
}

inline void PPU::WriteVertical( unsigned char Byte )
{
	m_ScrollHolding = 
		((Byte & 0x7) << 12) |
		((Byte & 0xF8) << 2) |
		(0x0C1F & m_ScrollHolding);
}

inline void PPU::WriteNameTable( unsigned char Byte )
{
	m_ScrollHolding = (m_ScrollHolding & 0xF3FF) | ((Byte<<10) & 0x0C00);
}

void PPU::WritePaletteRam( unsigned short Address, unsigned char Byte )
{
	Address &= 0x1F;
	m_Palette[m_PaletteConvertBus[Address]] = (Byte & 0x3F);
}

bool PPU::RequestNMI()
{
	return m_NMIActive && m_GenerateNMI;
}

unsigned char PPU::ReadPaletteRam( unsigned short Address )
{
	Address &= 0x1F;
	return m_Palette[m_PaletteConvertBus[Address]];
}

/* PPU CPU Visible Registers */
void PPU::PPUWrite( unsigned short Address, unsigned char Byte )
{
	switch( Address & 7 )
	{
	case 0:	// PPU CONTROL
		WriteNameTable( Byte );
		m_VRAMIncrement			= (Byte & 0x04) ? 32 : 1;
		m_OBJPattern			= (Byte & 0x08) ? 0x1000 : 0;
		m_PFPattern				= (Byte & 0x10) ? 0x1000 : 0;
		m_SpriteHeight			= (Byte & 0x20) ? 16 : 8;
		m_GenerateNMI			= (Byte & 0x80) ? true : false;

		break ;
	case 1:	// PPU MASK
		// Color specific conversions
		m_ColorMask	= (Byte & 0x01) ? 0x30 : 0x3F;
		m_Intensify = (Byte & 0xE0) << 1;

		// Rendering masks
		m_PFNoClip	= Byte & 0x02;
		m_OAMNoClip	= Byte & 0x04;
		m_PFEnable	= Byte & 0x08;
		m_OAMEnable = Byte & 0x10;

		break ;
	case 3:	// OAM ADDRESS PORT
		m_OAMAddress = Byte;
		break ;
	case 4: // OAM DATA PORT
		m_OAMMemory[m_OAMAddress++] = Byte;
		break ;
	case 5:	// PPU SCROLL REGISTERS
		// VERTICAL SCROLL
		if( m_HiLoToggle )
			WriteVertical( Byte );
		// HORIZONTAL SCROLL
		else
			WriteHorizontal( Byte );

		m_HiLoToggle = !m_HiLoToggle;
		break ;
	case 6:	// PPU VIDEO MEM ADDRESS PORT
		// ADDRESS LOW
		if( m_HiLoToggle )
		{
			m_VRAMAddress = m_ScrollHolding = (m_ScrollHolding & 0xFF00) | Byte;
			VideoAddressLatch( m_VRAMAddress );
		}
		// ADDRESS HIGH
		else
			m_ScrollHolding = (m_ScrollHolding & 0x00FF) | ((Byte << 8) & 0x3F00);

		m_HiLoToggle = !m_HiLoToggle;
		break ;
	case 7:	// PPU VIDEO MEM DATA PORT
		if( (m_VRAMAddress & 0x3F00) == 0x3F00 )
			WritePaletteRam( m_VRAMAddress, Byte );
		else
			// THIS COULD CAUSE ISSUES IF IT HAPPENS DURING ACTIVE DISPLAY!
			VideoWrite( Byte );

		m_VRAMAddress += m_VRAMIncrement;
		break ;
	}
}

unsigned char PPU::PPURead( unsigned short Address, unsigned char Trash )
{
	switch( Address & 7 )
	{
	case 0x2:	// STATUS DATA PORT
		{
			unsigned char r = (Trash & 0x1F) | m_VBlank | m_Sprite0Hit | m_SpriteOverflow;

			m_SupressVBL = true;
			m_VBlank = 0;
			m_HiLoToggle = false;

			if( m_NMISupressable > 0 )
				m_NMIActive = false;
			m_NMIDelay = false;

			return r;
		}
	case 0x4:	// OAM DATA PORT
		return m_OAMBuffer;
	case 0x7:	// VIDEO MEM DATA PORT
		m_NeedVideoRead = true;

		if( (m_VRAMAddress & 0x3F00) == 0x3F00 )
			return ReadPaletteRam( m_VRAMAddress );
		else
			return m_VideoMemoryBuffer;

		break ;
	}

	return Trash;
}

inline unsigned short PPU::IncrementHorizontal( unsigned short addr )
{
	unsigned short holding = (addr+1) & 0x1F;

	if( holding == 0 )
		addr ^= 0x0400;

	return (addr & 0x7FE0) | holding;
}

inline unsigned short PPU::IncrementVertical( unsigned short addr )
{
	// Vertical scroll masks back in old values 
	addr = ((addr + 0x1000)& 0x7BE0) | (m_ScrollHolding & 0x041F);	

	if( (addr & 0x7000) == 0 )
	{
		unsigned short holding = (addr+32) & 0x3E0;

		if( holding == 960 )
		{
			holding = 0;
			addr ^= 0x0800;
		}

		return (addr & 0x7C1F) | holding;
	}

	return addr;
}

inline void PPU::ClockPlayfield( int hpos, int vpos )
{
	// RENDER TIME PLAYFIELD READS
	if( hpos < 256 )
	{
		int renderPos = (hpos & 0xF8) + 16;
		unsigned char byte;

		switch( hpos & 7 )
		{
		case 0:
			VideoAddressLatch( (m_VRAMAddress & 0xFFF) | 0x2000 );
			break ;
		case 1:	// Nametable byte read			
			m_PatternAddress = (VideoRead() << 4) | m_PFPattern;
			break ;
		case 2:
			VideoAddressLatch( 0x23C0 |
				((m_VRAMAddress >> 2) & 0x07) |
				((m_VRAMAddress >> 4) & 0x38) |
				((m_VRAMAddress) & 0xC00) );
			break ;
		case 3:	// Attribute byte read
			// Read the attribute byte
			byte = VideoRead();

			byte = ((byte >> ((m_VRAMAddress & 2) | ((m_VRAMAddress >> 4) & 0x4))) & 3) << 2;

			for( int i = 0; i < 8; i++ )
				m_PFSerializer[renderPos++] = byte;

			break ;
		case 4:
			VideoAddressLatch(  m_PatternAddress | (m_VRAMAddress >> 12) );
			break ;
		case 5: // Pattern byte 0 read
			byte = VideoRead();

			for( int b = 0; b < 8; b++, renderPos++ )
				if( byte & (0x80 >> b) )
					m_PFSerializer[renderPos] |= 0x01;

			break ;
		case 6:
			VideoAddressLatch( m_PatternAddress | (m_VRAMAddress >> 12) | 0x08 );
			break ;
		case 7:	// Pattern byte 1 read
			byte = VideoRead();

			for( int b = 0; b < 8; b++, renderPos++ )
				if( byte & (0x80 >> b) )
					m_PFSerializer[renderPos] |= 0x02;

			m_VRAMAddress = IncrementHorizontal(m_VRAMAddress);
			break ;
		}		
	}
	// OAM PATTERN READS
	else if( hpos < 320 )
	{
		if( hpos == 257 )
		{
			if( vpos )
			{
				m_VRAMAddress = IncrementVertical(m_VRAMAddress);
			}
			else
			{
				// UPDATEING SCROLLIES ON SCANLINE 0
				m_VRAMAddress = m_ScrollHolding;
			}
		}

		int sprite = (hpos & 0x38) >> 3;

		switch( hpos & 7 )
		{
		case 0:	// Junk nametable
		case 2:
			if( m_SpriteHeight == 8 )
				VideoAddressLatch( (m_VRAMAddress & 0xFFF) | 0x2000 );
			else
				VideoAddressLatch( (m_VRAMAddress & 0xFFF) | 0x2000 );
			break ;
		case 1:	
		case 3:	// Junk nametable
			VideoRead();
			break ;
		case 4:
			if( m_SpriteHeight == 8 )
				VideoAddressLatch( Sprites[sprite].Pattern | m_OBJPattern );
			else
				VideoAddressLatch( Sprites[sprite].Pattern | Sprites[sprite].Page );

			break ;
		case 5: // OAM Pattern 0
			Sprites[sprite].Pat0 = VideoRead();
			break ;
		case 6:
			if( m_SpriteHeight == 8 )
				VideoAddressLatch( Sprites[sprite].Pattern | m_OBJPattern | 8 );
			else
				VideoAddressLatch( Sprites[sprite].Pattern | Sprites[sprite].Page | 8 );
			break ;
		case 7:	// OAM Pattern 1
			Sprites[sprite].Pat1 = VideoRead();
			break ;
		}
	}
	// BEGINNING SCANLINE READS
	else if( hpos < 336 )
	{
		int renderPos = hpos & 0x08;
		unsigned char byte;

		switch( hpos & 7 )
		{
		case 0:
			VideoAddressLatch( (m_VRAMAddress & 0xFFF) | 0x2000 );
			break ;
		case 1:	// Nametable byte read			
			m_PatternAddress = (VideoRead() << 4) | m_PFPattern;
			break ;
		case 2:
			VideoAddressLatch( 0x23C0 |
				((m_VRAMAddress >> 2) & 0x07) |
				((m_VRAMAddress >> 4) & 0x38) |
				((m_VRAMAddress) & 0xC00) );
			break ;
		case 3:	// Attribute byte read
			// Read the attribute byte
			byte = VideoRead();
			byte = ((byte >> ((m_VRAMAddress & 2) | ((m_VRAMAddress >> 4) & 0x4))) & 3) << 2;

			for( int i = 0; i < 8; i++ )
				m_PFSerializer[renderPos++] = byte;

			break ;
		case 4:
			VideoAddressLatch( m_PatternAddress | (m_VRAMAddress >> 12) );
			break ;
		case 5: // Pattern byte 0 read
			byte = VideoRead();

			for( int i = 0; i < 8; i++,renderPos++ )
				if( byte & (0x80 >> i) )
					m_PFSerializer[renderPos] |= 0x01;
			break ;
		case 6:
			VideoAddressLatch( m_PatternAddress | (m_VRAMAddress >> 12) | 0x08 );
			break ;
		case 7:	// Pattern byte 1 read
			byte = VideoRead();

			for( int i = 0; i < 8; i++,renderPos++ )
				if( byte & (0x80 >> i) )
					m_PFSerializer[renderPos] |= 0x02;

			m_VRAMAddress = IncrementHorizontal(m_VRAMAddress);
			break ;
		}		
	}
	// GARBAGE READS CYCLES
	else	
	{
		if( ~hpos & 1 )
			VideoAddressLatch( (m_VRAMAddress & 0xFFF) | 0x2000 );
		else
			VideoRead();
	}
}

const unsigned char SecondaryReadback[64] = {
		0, 1, 2, 3, 3, 3, 3, 3,
		4, 5, 6, 7, 7, 7, 7, 7,
		8, 9,10,11,11,11,11,11,
		12,13,14,15,15,15,15,15,
		16,17,18,19,19,19,19,19,
		20,21,22,23,23,23,23,23,
		24,25,26,27,27,27,27,27,
		28,29,30,31,31,31,31,31 };

inline void PPU::ClockOAM( int hpos, int vpos )
{
	// Secondary buffer clear
	if( hpos < 64 )
	{
		m_OAMBuffer = m_OAMSeconary[hpos>>1] = 0xFF;
	}
	// Sprite evaluation
	else if (hpos < 256)
	{
		int holding;

		if( ~hpos & 1 )
		{
			if( hpos == 64 )
			{
				m_SpritesFound = 0;
				m_OAMLoadLocation = 0;
				m_SecondaryLocation = 0;
				m_OAMLoadState = COMPARE_Y;

				for( int i = 0; i < 8; i++ )
				{
					Sprites[i].X = 240;
					Sprites[i].Pattern = 0;	// I don't know what this actually reads out as
					Sprites[i].Page = 0x1000;
				}
			}

			m_OAMBuffer = m_OAMMemory[m_OAMLoadLocation];
			return ;
		}

		switch( m_OAMLoadState )
		{
			/* Standard mode decode */
			case COMPARE_Y:
				m_OAMBuffer =  m_OAMMemory[m_OAMLoadLocation];
				holding = vpos - m_OAMBuffer - 1;
				
				// Value is not in range
				if( holding < 0 || holding >= m_SpriteHeight )
				{
					m_OAMLoadLocation += 4;

					if( m_OAMLoadLocation == 0 )
						m_OAMLoadState = PRIMARY_OVERFLOW;

					break ;
				}

				m_OAMSeconary[m_SecondaryLocation++] = m_OAMBuffer;

				Sprites[m_SpritesFound].Zero = (m_OAMLoadLocation == (m_OAMAddress&0xFC));
				Sprites[m_SpritesFound].Y = holding;
				m_OAMLoadLocation++;

				m_OAMLoadState = LOAD_TILE;
				break ;
			case LOAD_TILE:
				Sprites[m_SpritesFound].Tile = m_OAMBuffer =  m_OAMMemory[m_OAMLoadLocation++];
				m_OAMSeconary[m_SecondaryLocation++] = m_OAMBuffer;
				m_OAMLoadState = LOAD_ATTRIBUTES;
				break;
			case LOAD_ATTRIBUTES:
				m_OAMBuffer =  m_OAMMemory[m_OAMLoadLocation++];
				m_OAMSeconary[m_SecondaryLocation++] = m_OAMBuffer;

				Sprites[m_SpritesFound].HorizontalFlip = (m_OAMBuffer & 0x40) != 0;
				holding = (m_OAMBuffer & 0x80) ? (m_SpriteHeight - Sprites[m_SpritesFound].Y - 1) : Sprites[m_SpritesFound].Y;

				Sprites[m_SpritesFound].Mask =
					(m_OAMBuffer & 0x20) |						// Priority flag
					((m_OAMBuffer << 2) & 0x0C) |				// Palette
					(Sprites[m_SpritesFound].Zero ? 0x10 : 0);	// Zero flag

				if( m_SpriteHeight == 8 )
				{
					Sprites[m_SpritesFound].Pattern = 
						(Sprites[m_SpritesFound].Tile << 4)	|	// Tile * 16
						holding;								// Y Coordinate with flipping
				}			
				else
				{
					int tile = Sprites[m_SpritesFound].Tile;
					
					if( holding >= 8 )
						holding += 8;

					Sprites[m_SpritesFound].Pattern = holding | ((tile&0xFE)<<4 );
					Sprites[m_SpritesFound].Page    = ((tile&1) << 12);
				}

				m_OAMLoadState = LOAD_X;
				break ;
			case LOAD_X:
				m_OAMBuffer =  m_OAMMemory[m_OAMLoadLocation++];
				m_OAMSeconary[m_SecondaryLocation++] = m_OAMBuffer;
				Sprites[m_SpritesFound++].X = m_OAMBuffer;

				if( m_OAMLoadLocation == 0 )
					m_OAMLoadState = PRIMARY_OVERFLOW;
				else if( m_SpritesFound < 8 )
					m_OAMLoadState = COMPARE_Y;
				else
					m_OAMLoadState = COMPARY_Y_BUGGED;

				break ;

			/* Less than 8 sprites have been found, loop to infinity */
			case PRIMARY_OVERFLOW:
				m_OAMBuffer = m_OAMMemory[m_OAMLoadLocation];
				m_OAMLoadLocation += 4;
				break ;
			
			/* 8 sprites have been found, Start with the buggy comparison */
			case MOCK_COPY1:
			case MOCK_COPY2:
			case MOCK_COPY3:
				m_OAMBuffer =  m_OAMMemory[m_OAMLoadLocation++];
				m_OAMLoadState++;
				break ;
			case COMPARY_Y_BUGGED:
				m_OAMBuffer =  m_OAMMemory[m_OAMLoadLocation];
				holding = vpos - m_OAMBuffer - 1;

				// Value is not in range
				if( holding < 0 || holding >= m_SpriteHeight )
				{
					m_OAMLoadLocation = (((m_OAMLoadLocation+4) & 0xFC) | ((m_OAMLoadLocation+1)&3));

					if( m_OAMLoadLocation < 4 )
						m_OAMLoadState = PRIMARY_OVERFLOW;

					break ;
				}
				else
				{
					m_OAMLoadLocation++;
					m_SpriteOverflow = OVERFLOW_OAM_VALUE;
					m_OAMLoadState = MOCK_COPY1;
				}
				break ;
		}
	}

	// Sprite readback
	else if (hpos < 320)
	{
		m_OAMBuffer = m_OAMSeconary[SecondaryReadback[hpos&0x3F]];
	}
	// Garbage bytes
	else
	{
		if( m_SpritesFound == 0 )
			m_OAMBuffer = m_OAMMemory[252];	// Y of the sprite #63
		else
			m_OAMBuffer = m_OAMSeconary[0];	// First byte of secondary memory
	}
}

void PPU::RenderOAM()
{
	memset( m_OBJSerializer, 0, sizeof(m_OBJSerializer) );

	for( int i = m_SpritesFound-1; i >= 0; i-- )
	{		
		int x = Sprites[i].X;
		int mask = Sprites[i].Mask;
		int p0 = Sprites[i].Pat0;
		int p1 = Sprites[i].Pat1 << 1;
		unsigned char clr;

		if( Sprites[i].HorizontalFlip )
		{
			for( int b = 0; b <= 7; b++, x++ )
			{
				clr = ((p0 >> b) & 1) |	((p1 >> b) & 2) | mask;
				
				if( clr & 3 )
					m_OBJSerializer[x] = clr;
			}				
		}
		else
		{
			for( int b = 7; b >= 0; b--, x++ )
			{
				clr = ((p0 >> b) & 1) | ((p1 >> b) & 2) | mask;
				if( clr & 3 )
					m_OBJSerializer[x] = clr;
			}
		}
	}

	m_OBJSerializer[0xFF] = 0;
}

unsigned short* PPU::VideoClock()
{
	m_NMIActive = m_NMIDelay;
	if( m_NMISupressable )
		m_NMISupressable--;

	// TODO: HANDLE BAD THINGS DURING ACTIVE DISPLAY
	if( m_HPos & 1 )
	{
		if( m_NeedVideoRead )
		{
			m_VideoMemoryBuffer = VideoRead();
			m_VRAMAddress += m_VRAMIncrement;
			m_NeedVideoRead = false;
		}
	}

	// ACTIVE DISPLAY PERIOD
	if( m_VPos < 241 )
	{
		if( m_HPos == 340 )
		{
			RenderOAM();

			m_VPos++;
			m_HPos = 0;
		}
		else
		{
			if( m_PFEnable || m_OAMEnable )
			{
				if( m_HPos < 256 )
				{
					unsigned char bg, spr;

					if( m_HPos < 8 )
					{
						bg  = (m_PFEnable && m_PFNoClip) ? 
							m_PFSerializer[m_HPos + m_ScrollHorizontal] : 0;
						spr = (m_OAMEnable && m_OAMNoClip) ? 
							m_OBJSerializer[m_HPos] : 0;
					}
					else
					{
						bg	= m_PFEnable ? 
							m_PFSerializer[m_HPos + m_ScrollHorizontal] : 0;
						spr = m_OAMEnable ? 
							m_OBJSerializer[m_HPos] : 0;
					}

					unsigned char color = m_PriorityEncoder[ (spr << 4) | bg ];

					m_Sprite0Hit |= color & 0x40;

					*(m_Pixel++) = (m_Palette[color&0x1F] & m_ColorMask) | m_Intensify;
				}

				ClockPlayfield( m_HPos, m_VPos );
				ClockOAM( m_HPos, m_VPos );
			}
			else
			{
				if( m_HPos < 256 )
					*(m_Pixel++) = (m_Palette[0] & m_ColorMask) | m_Intensify;
	
				if( ~m_HPos & 1 )
				{
					VideoAddressLatch( m_VRAMAddress );
				}				
			}

			m_HPos++;
		}
	}
	// PRE-INTERRUPT BLANKING
	else if( m_VPos == 241 )
	{
		if( ~m_HPos & 1 )
			VideoAddressLatch( m_VRAMAddress );

		m_OAMBuffer = m_OAMMemory[m_OAMAddress];
		
		if( ++m_HPos == 341 )
		{
			if( !m_SupressVBL )
			{
				m_VBlank = VBLANK_VALUE;
				m_NMIDelay = true;	
				m_NMIActive = true;
			}
			
			m_NMISupressable = 2;

			m_VPos++;
			m_HPos = m_VerticalBlankPeriod;

			// Return our frame buffer one line late (first line is garbage)
			return m_Frame + PPU_PITCH;
		}
		m_SupressVBL = false;
	}
	else
	{
		if( ~m_HPos & 1 )
			VideoAddressLatch( m_VRAMAddress );

		m_OAMBuffer = m_OAMMemory[m_OAMAddress];
		
		m_HPos--;

		// Dropped cycle
		if( m_HPos == 13 && m_FrameToggle && m_PFEnable )
		{
			m_HPos--;
		}
		else if( m_HPos == 0 )
		{
			m_Pixel = m_Frame;
			m_VPos = 0;
			m_Sprite0Hit = 0;
			m_SpriteOverflow = 0;
			m_VBlank = 0;
			m_NMIActive = m_NMIDelay = false;

			m_FrameToggle = !m_FrameToggle;
		}
	}

	return NULL;
}
