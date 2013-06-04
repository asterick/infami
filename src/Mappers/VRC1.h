#ifndef __VRC1_H
#define __VRC1_H

class VRC1 : public ROM
{
public:
	VRC1( const TimingSettings *Timing ) : ROM( Timing ) {}
	Famicom *GetSystem() { return (Famicom*) this; }
	virtual void Initalize( int Mirroring );
protected:
	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	virtual void VideoWrite(  unsigned char Byte );
	virtual unsigned char VideoRead();
	//virtual bool IRQActive();


	unsigned char	m_BusTrash;

	unsigned short	m_VideoAddress;

	int				m_PRGMask;
	int				m_CHRMask;

	int				m_PRGPage0;
	int				m_PRGPage1;
	int				m_PRGPage2;

	int				m_CHRPage0;
	int				m_CHRPage1;
};

#endif