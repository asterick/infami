#ifndef __VRC7_H
#define __VRC7_H

class VRC7 : public ROM
{
public:
	VRC7( const TimingSettings *Timing );
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

	int				ChrMask;

	int				m_PRGPage0;
	int				m_PRGPage1;
	int				m_PRGPage2;

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

#endif