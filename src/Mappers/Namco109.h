#ifndef __NAMCO109_H
#define __NAMCO109_H

class Namco109 : public ROM
{
public:
	Namco109( const TimingSettings *Timing ) : ROM( Timing ) {}
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

	int m_PageSelect;
	int m_PRGPage0;
	int m_PRGPage1;
	int m_CHRPage0;
	int m_CHRPage1;
	int m_CHRPage2;
	int m_CHRPage3;

	unsigned short	m_VideoAddress;

	unsigned char	m_BusTrash;
};

#endif