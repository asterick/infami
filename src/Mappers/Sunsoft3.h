#ifndef __SUNSOFT3_H
#define __SUNSOFT3_H

class Sunsoft3 : public ROM
{
public:
	Sunsoft3( const TimingSettings *Timing ) : ROM( Timing ) {}
	virtual void Initalize( int Mirroring );
protected:
	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	virtual void VideoWrite(  unsigned char Byte );
	virtual unsigned char VideoRead();
	
	virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }
private:
	void CPUClock();

	unsigned char	m_BusTrash;

	unsigned short	m_VideoAddress;

	int				m_PRGMask;
	int				m_CHRMask;
	int				m_PRGBank;
	int				m_CHRBank0;
	int				m_CHRBank1;
	int				m_CHRBank2;
	int				m_CHRBank3;

	bool			m_IRQEnable;
	bool			m_IRQActive;
	bool			m_WriteLo;
	unsigned short	m_Counter;
	unsigned short	m_CounterHolding;
};

#endif