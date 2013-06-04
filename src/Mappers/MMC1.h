#ifndef __MMC1_H
#define __MMC1_H

class MMC1 : public ROM
{
public:
	MMC1( const TimingSettings *Timing ) : ROM( Timing ) {}

	unsigned char Read( unsigned short addr );
	void Write( unsigned short addr, unsigned char ch );

	virtual void VideoAddressLatch( unsigned short Address );
	void VideoWrite(  unsigned char Byte );
	unsigned char VideoRead();

	Famicom *GetSystem() { return (Famicom*) this; }
	virtual void Initalize( int Mirroring );
private:
	void SetBanks();
	void WriteShifting( int bank, unsigned char ch );

	int m_PRGPageLo;
	int m_PRGPageHi;
	int m_CHRPageLo;
	int m_CHRPageHi;

	int m_CHRMask;
	int m_PRGMask;

	unsigned char ShiftReg;
	int Bits;
	
	int m_ChrPage0;
	int m_ChrPage1;
	int m_PrgPage;

	bool m_WriteProtect;
	bool m_PrgBank;
	bool m_16kPRG;
	bool m_4kCHR;
	bool m_Ignore;

	unsigned short	m_VideoAddress;

	unsigned char	m_BusTrash;
};

#endif