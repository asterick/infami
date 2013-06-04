#ifndef __VRC3_H
#define __VRC3_H

class VRC3 : public ROM
{
public:
	VRC3( const TimingSettings *Timing ) : ROM( Timing ) {}
	Famicom *GetSystem() { return (Famicom*) this; }
protected:
	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	virtual void VideoWrite(  unsigned char Byte );
	virtual unsigned char VideoRead();
	
	virtual bool IRQActive();
	virtual void Initalize( int Mirroring );
private:
	void CPUClock();

	unsigned char	m_BusTrash;

	int				m_PRGMask;
	int				m_PRGPage;
	
	int				m_CounterReload;
	int				m_Counter;

	unsigned short	m_VideoAddress;

	bool			m_IRQEnaAck;
	bool			m_IRQEnable;
	bool			m_IRQMode;
	bool			m_IRQActive;
};

#endif