#ifndef __MMC2_H
#define __MMC2_H

class MMC2 : public ROM
{
public:
	MMC2( const TimingSettings *Timing ) : ROM( Timing ) {}
	virtual void Initalize( int Mirroring );
protected:
	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	virtual void VideoWrite(  unsigned char Byte );
	virtual unsigned char VideoRead();
	//virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }

	unsigned char	m_BusTrash;
	unsigned short	m_VideoAddress;

	int				m_PRGPage;
	int				m_CHRPage0;
	int				m_CHRPage1;
	int				m_CHRPage2;
	int				m_CHRPage3;
	bool			m_ChrLoLatch;
	bool			m_ChrHiLatch;

	int				m_PRGMask;
	int				m_PRGTop;
	int				m_CHRMask;
};

#endif