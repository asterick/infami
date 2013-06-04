#ifndef __CAMERICA_H
#define __CAMERICA_H

class Camerica : public NROM
{
public:
	Camerica( const TimingSettings *Timing ) : NROM( Timing ) {}
	virtual void Initalize( int Mirroring );
protected:
	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoWrite(  unsigned char Byte );
	virtual unsigned char VideoRead();
	//virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }

	int m_PRGMask;
	int m_PRGBank;
	int m_NamePage;
};

#endif