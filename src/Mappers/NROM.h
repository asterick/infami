#ifndef __NROM_H
#define __NROM_H

class NROM : public ROM
{
public:
	NROM( const TimingSettings *Timing ) : ROM( Timing ) {}
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
};

#endif