#ifndef __NAMCOT106_H
#define __NAMCOT106_H

class Namcot106 : public ROM
{
public:
	Namcot106( const TimingSettings *Timing ) : ROM( Timing ) {}
	virtual void Initalize( int Mirroring );
protected:
	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	virtual void VideoWrite(  unsigned char Byte );
	virtual unsigned char VideoRead();
	virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }

	void RegisterWrite( unsigned short addr, unsigned char ch );
	unsigned char RegisterRead( unsigned short addr );
	void AudioUpdate( int channel );
	
	float StreamAudio();
	void CPUClock();

	unsigned char	m_BusTrash;
	unsigned short	m_VideoAddress;

	unsigned char	m_NameTables[0x2000];

	bool			m_IRQActive;
	bool			m_IRQEnable;
	int				m_Counter;

	int				ChrMask;

	int				m_LowPageRam;
	int				m_HighPageRam;

	unsigned char	m_CHRBank[12];
	unsigned char*	m_CHRPage[12];
	bool			m_CHRProtect[12];

	unsigned char*	m_PRGPage0;
	unsigned char*	m_PRGPage1;
	unsigned char*	m_PRGPage2;

	// --- FME-7 AUDIO REGISTERS ---
	int				m_AudioAddress;
	bool			m_AudioIncrement;
	int				m_AudioRam[0x80];
	int				m_WaveTable[0x100];

	struct WaveChannel
	{
		int m_Frequency;
		int m_WaveAddress;
		int m_WaveLength;
		int	m_Volume;

		int m_CurrentAddress;
		int m_BytesLeft;

		int m_Error;
		int m_Output;
	};

	WaveChannel		m_WaveChannel[8];
	int				m_AudioSamples;
	float			m_AudioAccumulator;
	int				m_ActiveChannels;
	int				m_OutputChannel;
};

#endif