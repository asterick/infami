#ifndef __BXNINA_H
#define __BXNINA_H

class BxNINA : public ROM
{
public:
	BxNINA( const TimingSettings *Timing ) : ROM( Timing ) {}
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
	int	m_CHR0Page;
	int	m_CHR1Page;

	int m_PRGMask;
	int m_CHRMask;

	unsigned short	m_VideoAddress;

	unsigned char	m_BusTrash;
};

#endif