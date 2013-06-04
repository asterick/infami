#ifndef __GXROM_H
#define __GXROM_H

class GxROM : public NROM
{
public:
	GxROM( const TimingSettings *Timing ) : NROM(Timing) {}
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