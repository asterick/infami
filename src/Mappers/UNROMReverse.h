#ifndef __UNROMREVERSE_H
#define __UNROMREVERSE_H

class UNROMReverse : public NROM
{
public:
	UNROMReverse( const TimingSettings *Timing ) : NROM(Timing) {}
	Famicom *GetSystem() { return (Famicom*) this; }
	virtual void Initalize( int Mirroring );
protected:
	unsigned char Read( unsigned short addr );
	void Write( unsigned short addr, unsigned char ch );
private:
	unsigned char *m_Page;
	int	m_PageMask;
	int	m_TopPage;
};

#endif