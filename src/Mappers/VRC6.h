#ifndef __VRC6_H
#define __VRC6_H

class VRC6 : public ROM
{
public:
	VRC6( const TimingSettings *Timing, int bit0, int bit1 );
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

	virtual float StreamAudio();

	unsigned char	m_BusTrash;
	unsigned short	m_VideoAddress;

	int				m_CHRMask;
	int				PrgMaskLo;
	int				PrgMaskHi;

	int				m_Bit1;
	int				m_Bit0;

	int				m_PRGPage0;
	int				m_PRGPage1;

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

	// -- Audio channel Variables --
	float			m_AudioAccumulator;
	int				m_AudioSamples;

	bool			m_Ch1Enable;
	bool			m_Ch2Enable;
	bool			m_Ch3Enable;
	int				m_Ch1Freq;
	int				m_Ch2Freq;
	int				m_Ch3Freq;
	int				m_Ch1Overflow;
	int				m_Ch2Overflow;
	int				m_Ch3Overflow;
	int				m_Ch1Sequencer;
	int				m_Ch2Sequencer;
	int				m_Ch3Sequencer;

	int				m_Ch1Volume;
	int				m_Ch2Volume;
	unsigned short	m_Ch1Duty;
	unsigned short	m_Ch2Duty;

	int				m_Ch3AccumRate;
	unsigned char	m_Ch3Accumulator;

	int				m_Ch1Output;
	int				m_Ch2Output;
	int				m_Ch3Output;
};

class VRC6a : public VRC6
{
public:
	VRC6a( const TimingSettings *Timing ) :
		VRC6( Timing,  1,  2 ) {}
};

class VRC6b : public VRC6
{
public:
	VRC6b( const TimingSettings *Timing ) :
		VRC6( Timing, 2, 1 ) {}
};

#endif