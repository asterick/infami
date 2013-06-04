#ifndef __COLORDREAMS_H
#define __COLORDREAMS_H

class ColorDreams : public NROM
{
public:
	ColorDreams( const TimingSettings *Timing ) : NROM(Timing) {}
	virtual void Initalize( int Mirroring );
protected:
	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );
	
	virtual unsigned char VideoRead();
	//virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }

	int m_PRGMask;
	int m_CHRMask;
	int m_PRGBank;
	int m_CHRBank;
};

#endif