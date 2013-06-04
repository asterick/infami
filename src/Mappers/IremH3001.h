#ifndef __IREMH3001_H
#define __IREMH3001_H

class IremH3001 : public ROM
{
public:
	IremH3001( const TimingSettings *Timing ) : ROM( Timing ) {}
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
	void CPUClock();
	bool m_PRGLockLo;
	bool m_CHRSwapLo;
	
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

	bool m_IRQEnable;
	bool m_IRQActive;

	int m_IRQCounter;
	int m_IRQReload;
};

#endif