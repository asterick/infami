#ifndef __AXROM_H
#define __AXROM_H

class AxROM : public ROM
{
public:
	AxROM( const TimingSettings *Timing ) : ROM( Timing ) {}
	virtual void Initalize( int Mirroring );
protected:
	unsigned char Read( unsigned short addr );
	void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	void VideoWrite( unsigned char Byte );
	unsigned char VideoRead();
	//virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }
private:	
	int	m_PRGPage;
	int m_PRGMask;

	unsigned short	m_VideoAddress;
	unsigned char	m_BusTrash;
};

#endif