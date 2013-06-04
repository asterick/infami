#ifndef __VROMPROTECT_H
#define __VROMPROTECT_H

class VROMProtect : public NROM
{
public:
	VROMProtect( const TimingSettings *Timing ) : NROM(Timing) {}
	virtual void Initalize( int Mirroring );
protected:
	void Write( unsigned short addr, unsigned char ch );

	unsigned char VideoRead();
	Famicom *GetSystem() { return (Famicom*) this; }

private:
	bool			m_Disable;
};

#endif