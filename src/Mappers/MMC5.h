#ifndef __MMC5_H
#define __MMC5_H

enum PRG_MODES
{
	MMC5_PRGMODE_32K,
	MMC5_PRGMODE_16K,
	MMC5_PRGMODE_MIXED,
	MMC5_PRGMODE_8K
};

enum CHR_MODES
{
	MMC5_CHRMODE_8K,
	MMC5_CHRMODE_4K,
	MMC5_CHRMODE_2K,
	MMC5_CHRMODE_1K
};

enum EXRAM_MODES
{
	MMC5_EXRAM_NAMETABLE,
	MMC5_EXRAM_ATTRCHREXT,
	MMC5_EXRAM_RAM,
	MMC5_EXRAM_RAM_READONLY,
};

class MMC5 : public ROM
{
public:
	MMC5( const TimingSettings *Timing );

	float StreamAudio();
protected:
	virtual void Initalize( int Mirroring );

	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );

	virtual void CPUClock();

	virtual void VideoAddressLatch( unsigned short Address );
	virtual void VideoWrite(  unsigned char Byte );
	virtual unsigned char VideoRead();
	virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }

	unsigned char	m_BusTrash;
	unsigned char	m_NameTable0[0x400];
	unsigned char	m_NameTable1[0x400];
	unsigned char	m_ExRam[0x400];
	unsigned short	m_VideoAddress;

private:
	unsigned char RegisterRead( unsigned short addr );
	void RegisterWrite( unsigned short addr, unsigned char ch );
	void PrgSetPage( int page, int bank, int mask );
	void PrgCopyPage( int page_dst, int page_src );
	void UpdatePRGBanks();
	void UpdateCHRBanks();
	void UpdateNameBanks();

	int					BckMask;
	int					WrkMask;
	int					ChrMask;

	// CPU Space bank switching

	int					m_PRGMode;
	int					m_PRGBank[5];
	unsigned char*		m_PRGPage[5];
	bool				m_PRGWrite[4];
	unsigned char*		m_WRKPage;
	bool				m_WRKProtect1;
	bool				m_WRKProtect2;

	// PPU BANK SWITCHING
	unsigned char*		m_SPRPage[16];
	unsigned char*		m_CHRPage[16];
	bool				m_FillMode[16];

	int					m_CHRMode;
	int					m_CHRExtender;
	int					m_SPRBank[8];
	int					m_CHRBank[4];

	// SPECIAL MODES
	int					m_NameTableMapping;
	int					m_ExRAMMode;

	unsigned char		m_Multiplier;
	unsigned char		m_Multiplicand;
	unsigned short		m_MultiResult;

	unsigned char		m_FillModeTile;
	unsigned char		m_FillModeAttr;
	int					m_ExAttribute;
	int					m_ExTile;

	bool				m_EnableVSplit;
	bool				m_RightVSplit;
	int					m_OffsetVSplit;
	int					m_ScrollVSplit;
	unsigned char*		m_BankVSplit;

	// IRQ Stuff
	int					m_HorizontalCounter;
	int					m_LineCounter;
	int					m_LineLatch;
	bool				m_IRQActive;
	bool				m_IRQEnable;
	bool				m_InFrameFlag;
	bool				m_InSprite;

	int					m_SyncPulses;

	// FRAME SENSING
	bool				m_ReadTimeout;

	// Audio registers
	SquareAPU	Square1;
	SquareAPU	Square2;
	int			m_PCMOutput1;
	int			m_PCMOutput2;
	int			m_PCMSelect;

	float		m_AudioAccumulator;
	int			m_AudioSamples;
	int			m_EnvelopeCount;
	bool		m_ClockLength;
};

#endif