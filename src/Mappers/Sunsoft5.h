#ifndef __SUNSOFT5_H
#define __SUNSOFT5_H

class Sunsoft5 : public ROM
{
public:
	Sunsoft5( const TimingSettings *Timing ) : ROM( Timing ) {}
	virtual void Initalize( int Mirroring );
protected:
	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	virtual void VideoWrite(  unsigned char Byte );
	virtual unsigned char VideoRead();
	
	virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }
	float StreamAudio();
private:
	void CPUClock();

	unsigned char	m_BusTrash;

	unsigned short	m_VideoAddress;

	int				m_PRGMask;
	int				m_CHRMask;

	unsigned char	m_BankRegister;
	
	unsigned char*	m_PRGBank0;
	unsigned char*	m_PRGBank1;
	unsigned char*	m_PRGBank2;
	unsigned char*	m_PRGBank3;

	bool			m_BankWritable;
	
	unsigned char*	m_CHRBank0;
	unsigned char*	m_CHRBank1;
	unsigned char*	m_CHRBank2;
	unsigned char*	m_CHRBank3;
	unsigned char*	m_CHRBank4;
	unsigned char*	m_CHRBank5;
	unsigned char*	m_CHRBank6;
	unsigned char*	m_CHRBank7;

	bool			m_IRQEnable;
	bool			m_IRQActive;
	bool			m_WriteLo;

	unsigned short	m_Counter;
	unsigned short	m_CounterHolding;

	// --- AY-3-8910 AUDIO ---
	int				m_AudioAddress;

	int				m_MasterDivider;
	float			m_AudioAccumulator;
	int				m_AudioSamples;

	int				m_CH1Period;
	int				m_CH2Period;
	int				m_CH3Period;

	int				m_CH1Overflow;
	int				m_CH2Overflow;
	int				m_CH3Overflow;

	int				m_CH1Volume;
	int				m_CH2Volume;
	int				m_CH3Volume;

	int				m_CH1Output;
	int				m_CH2Output;
	int				m_CH3Output;

	int				m_CH1Enable;
	int				m_CH2Enable;
	int				m_CH3Enable;

	int				m_CH1Noise;
	int				m_CH2Noise;
	int				m_CH3Noise;
};


#endif