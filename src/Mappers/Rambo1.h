#ifndef __RAMBO1_H
#define __RAMBO1_H

class Rambo1 : public ROM
{
public:
	Rambo1( const TimingSettings *Timing ) : ROM( Timing ) {}
	virtual void Initalize( int Mirroring );
protected:
	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	virtual void VideoWrite(  unsigned char Byte );
	virtual unsigned char VideoRead();

	void BuildPages();
	virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }

	unsigned char	m_BusTrash;

	unsigned char	m_Bank[16];

	unsigned short	m_VideoAddress;

	int				m_PRGMask;
	int				m_CHRMask;

	int				m_CHRPage0;
	int				m_CHRPage1;
	int				m_CHRPage2;
	int				m_CHRPage3;
	int				m_CHRPage4;
	int				m_CHRPage5;
	int				m_CHRPage6;
	int				m_CHRPage7;

	int				m_PRGPage0;
	int				m_PRGPage1;
	int				m_PRGPage2;

	int				m_PRGFlip;
	int				m_CHRFlip;
	int				m_1KMode;

	int				m_BankSelect;


	// IRQ Jazz
	void ClockVideo( int edge );
	void CPUClock();
	void ClockIRQ();

	bool m_IRQEnable;
	bool m_IRQActive;

	bool m_A12Edge;
	bool m_ReloadIRQ;
	bool m_CPUClockIRQ;
	int m_IRQCounter;
	int m_IRQReload;
	int m_EdgeTimeout;
	int m_ClockDivide;
};

#endif