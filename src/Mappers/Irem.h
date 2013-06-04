#ifndef __IREM_H
#define __IREM_H

class Irem : public ROM
{
public:
	Irem( const TimingSettings *Timing ) : ROM( Timing ) {}
	virtual void Initalize( int Mirroring );
protected:
	unsigned char Read( unsigned short addr );
	void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	void VideoWrite(  unsigned char Byte );
	unsigned char VideoRead();
	//virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }
private:	
	int m_PRGMask;
	int m_CHRMask;

	int m_CHRPage;
	int m_PRGPage;

	unsigned short	m_VideoAddress;

	unsigned char	m_BusTrash;
};

#endif