#ifndef __CXROM_H
#define __CXROM_H

class CxROM : public NROM
{
public:
	CxROM( const TimingSettings *Timing ) : NROM( Timing ) {}
	virtual void Initalize( int Mirroring );
protected:
//	unsigned char Read( unsigned short addr );
	void Write( unsigned short addr, unsigned char ch );
//	void VideoWrite(  unsigned char Byte );
	unsigned char VideoRead();
	//virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }
private:
	unsigned int	m_Page;
	unsigned int	m_CHRMask;
};

#endif