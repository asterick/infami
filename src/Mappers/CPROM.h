#ifndef __CPROM_H
#define __CPROM_H

class CPROM : public NROM
{
public:
	CPROM( const TimingSettings *Timing ) : NROM( Timing ) {}
	virtual void Initalize( int Mirroring );
protected:
//	unsigned char Read( unsigned short addr );
	void Write( unsigned short addr, unsigned char ch );
	
	void VideoWrite(  unsigned char Byte );
	unsigned char VideoRead();
	//virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }
private:
	int				m_CHRMask;
	unsigned short	m_Page;
};

#endif