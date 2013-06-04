#ifndef __VRC2_4_H
#define __VRC2_4_H

class VRC2_4 : public ROM
{
public:
	VRC2_4( const TimingSettings *Timing, int bit0, int bit1, bool halfChr );
	virtual void Initalize( int Mirroring );
protected:
	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	virtual void VideoWrite(  unsigned char Byte );
	virtual unsigned char VideoRead();
	virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }

	void WriteReg( unsigned short addr, unsigned char ch );
	void CPUClock();

	unsigned char	m_BusTrash;
	unsigned short	m_VideoAddress;

	int				m_CHRMask;
	int				m_NearLastPage;

	int				m_Bit1;
	int				m_Bit0;

	bool			m_PRGSwap;
	int				m_PRGPage0;
	int				m_PRGPage1;

	bool			m_HalfChar;
	int				m_CHRPage0;
	int				m_CHRPage1;
	int				m_CHRPage2;
	int				m_CHRPage3;
	int				m_CHRPage4;
	int				m_CHRPage5;
	int				m_CHRPage6;
	int				m_CHRPage7;

	int				m_CounterReload;
	int				m_Counter;
	int				m_Scalar;

	bool			m_IRQActive;
	bool			m_IRQEnaAck;
	bool			m_IRQEnable;
	bool			m_IRQMode;
};

class VRC2a : public VRC2_4
{
public:
	VRC2a( const TimingSettings *Timing ) :
		VRC2_4( Timing,  2,  1, true ) {}
};

class VRC2b : public VRC2_4
{
public:
	VRC2b( const TimingSettings *Timing ) :
		VRC2_4( Timing, 1+4, 2+8, false ) {}
};

class VRC4a : public VRC2_4
{
public:
	VRC4a( const TimingSettings *Timing ) :
		VRC2_4( Timing, 0x42, 0x84, false ) {}
};

class VRC4b : public VRC2_4
{
public:
	VRC4b( const TimingSettings *Timing ) :
		VRC2_4( Timing, 2+8, 1+4, false ) {}
};

#endif