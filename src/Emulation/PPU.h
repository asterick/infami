#ifndef __PPU_H
#define __PPU_H

#define PPU_PITCH	(256)		// 32 on screen tiles, 2 extra tiles + scroll shift
#define PPU_HEIGHT	(240)		// 240 vertical display lines
#define PPU_FRAME	(256*241)	// Total frame size

#define VBLANK_VALUE		0x80
#define SPRITE0_VALUE		0x40
#define OVERFLOW_OAM_VALUE	0x20

enum
{
	COMPARE_Y,
	LOAD_TILE,
	LOAD_ATTRIBUTES,
	LOAD_X,

	PRIMARY_OVERFLOW,
			
	MOCK_COPY1,
	MOCK_COPY2,
	MOCK_COPY3,
	COMPARY_Y_BUGGED
};		

class PPU
{
public:
	PPU(int verticalLines );

protected:
	unsigned short* VideoClock();
	
	void PPUWrite( unsigned short Address, unsigned char Byte );
	unsigned char PPURead( unsigned short Address, unsigned char Trash );
	bool RequestNMI();

	/* EXPECTED OVERRIDES */
	virtual void VideoAddressLatch( unsigned short Address ) {}
	virtual void VideoWrite( unsigned char Byte ) {}	
	virtual unsigned char VideoRead() { return 0; }

	unsigned char	GetNameTable( unsigned short Address );
	void			SetNameTable( unsigned short Address, unsigned char Byte );

	void			SetMirrorPage( int page, unsigned char *data );
	void			SetMirrorVertical();
	void			SetMirrorHorizontal();
	void			SetMirrorFourScreen();
	void			SetMirrorZero();
	void			SetMirrorOne();	

private:
	void WriteHorizontal( unsigned char Byte );
	void WriteVertical( unsigned char Byte );
	void WriteNameTable( unsigned char Byte );
	void WritePaletteRam( unsigned short Address, unsigned char Byte );
	unsigned char ReadPaletteRam( unsigned short Address );
	
	unsigned short IncrementHorizontal( unsigned short addr );
	unsigned short IncrementVertical( unsigned short addr );
	void ClockPlayfield( int hpos, int vpos );
	void ClockOAM( int hpos, int vpos );
	void RenderOAM();
	
	unsigned short	m_Frame[ PPU_PITCH * 241 ];
	unsigned char	m_Palette[32];
	unsigned char	m_PaletteConvertBus[0x20];
	unsigned short* m_Pixel;

	/* Serializer buffer */
	unsigned char	m_PriorityEncoder[0x400];
	unsigned char	m_PFSerializer[256+16];	
	unsigned char	m_OBJSerializer[256+16];
	unsigned short	m_PatternAddress;

	/* Display state values */
	int				m_HPos;
	int				m_VPos;
	bool			m_FrameToggle;
	int				m_VerticalBlankPeriod;	
	bool			m_HiLoToggle;

	/* Display scroll effective registers */
	unsigned short	m_Intensify;
	unsigned short	m_ColorMask;
	unsigned short	m_OBJPattern;
	unsigned short	m_PFPattern;
	int				m_PFNoClip;
	int				m_OAMNoClip;
	int				m_PFEnable;
	int				m_OAMEnable;
	int				m_VRAMIncrement;
	int				m_ScrollHorizontal;
	int				m_SpriteHeight;
	bool			m_GenerateNMI;

	/* Display flag flags */
	unsigned char	m_VBlank;
	unsigned char	m_Sprite0Hit;
	unsigned char	m_SpriteOverflow;
	bool			m_SupressVBL;
	bool			m_NMIActive;
	bool			m_NMIDelay;
	int				m_NMISupressable;

	/* OAM Memory */
	unsigned char	m_OAMAddress;
	unsigned char	m_OAMMemory[0x100];	// 512bytes of primary memory
	unsigned char	m_OAMSeconary[32];	// 32byte of secondary memory
	unsigned char	m_OAMBuffer;

	int				m_OAMLoadState;
	unsigned char	m_OAMLoadLocation;
	int				m_SpritesFound;
	unsigned char	m_SecondaryLocation;

	struct
	{
		unsigned char	Y;				// Temporary Values
		unsigned char	Tile;

		unsigned short	Pattern;		// Location of pattern's first byte
		unsigned short	Page;

		bool			Zero;
		bool			HorizontalFlip;
		unsigned char	X;
		unsigned char	Mask;			// 0x10 = always   0x20 = priority  0x0C = palette
		unsigned char	Pat0;
		unsigned char	Pat1;
	} Sprites[8];

	/* Video Memory */
	unsigned char	m_VideoMemoryBuffer;
	bool			m_NeedVideoRead;
	bool			m_NeedVideoLatch;
	unsigned short	m_ScrollHolding;
	unsigned short	m_VRAMAddress;

	// Name table
	unsigned char*	m_NameTables[4];
	unsigned char	m_NameTable0[0x400];
	unsigned char	m_NameTable1[0x400];
	unsigned char	m_NameTable2[0x400];
	unsigned char	m_NameTable3[0x400];
};

#endif