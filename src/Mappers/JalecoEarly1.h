#ifndef __JALECOEARLY1_H
#define __JALECOEARLY1_H

class JalecoEarly1 : public ROM
{
public:
	JalecoEarly1( const TimingSettings *Timing ) : ROM( Timing ) {}
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

	int				m_PRGMask;
	int				m_CHRMask;

	int				m_PRGLatch;
	int				m_CHRLatch;
	int				m_HoldingLatch;
};

#endif