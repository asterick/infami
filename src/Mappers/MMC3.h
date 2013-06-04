#ifndef __MMC3_H
#define __MMC3_H

class MMC3 : public ROM
{
public:
	MMC3( const TimingSettings *Timing ) : ROM( Timing ) {}
	virtual void Initalize( int Mirroring );
protected:
	unsigned char Read( unsigned short addr );
	void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	void VideoWrite(  unsigned char Byte );
	unsigned char VideoRead();
	virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }
private:
	void ClockIRQ( int edge );
	void UpdateBanks();

	bool m_IRQEnable;
	bool m_IRQActive;

	bool m_A12Edge;
	bool m_ReloadIRQ;
	int m_IRQCounter;
	int m_IRQReload;
	int m_EdgeTimeout;

	bool m_PRGLockLo;
	bool m_CHRSwapLo;
	int  m_BankSelect;
	int  m_Banks[8];
	
	int m_PRGMask;
	int m_CHRMask;

	int m_PRGBank0;
	int m_PRGBank1;
	int m_PRGBank2;

	int m_CHRBank0;
	int m_CHRBank1;
	int m_CHRBank2;
	int m_CHRBank3;
	int m_CHRBank4;
	int m_CHRBank5;
	int m_CHRBank6;
	int m_CHRBank7;

	unsigned short	m_VideoAddress;

	unsigned char	m_BusTrash;
};

#endif