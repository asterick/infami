#ifndef __BANDAI70_H
#define __BANDAI70_H

class Bandai70 : public ROM
{
public:
	Bandai70( const TimingSettings *Timing ) : ROM( Timing ) {}
	virtual void Initalize( int Mirroring );
protected:
	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	virtual void VideoWrite(  unsigned char Byte );
	virtual unsigned char VideoRead();
	//virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }

	unsigned char	m_BusTrash;
	unsigned short	m_VideoAddress;

	int				m_NameTable;
	int				m_PRGBank;
	int				m_CHRBank;
	int				ChrMask;

};

#endif