#ifndef __APU_H
#define __APU_H

const int LengthTable[32] = 
{
	0x0A,0xFE,
	0x14,0x02,
	0x28,0x04,
	0x50,0x06,
	0xA0,0x08,
	0x3C,0x0A,
	0x0E,0x0C,
	0x1A,0x0E,
	0x0C,0x10,
	0x18,0x12,
	0x30,0x14,
	0x60,0x16,
	0xC0,0x18,
	0x48,0x1A,
	0x10,0x1C,
	0x20,0x1E	
};

/* Delta modulation is extended for direct BUS access */

class APU : public DeltaModAPU
{
public:
	APU(  const TimingSettings* timing );

	void InsertController( bool Player2, Controller *ctrl );
protected:

	void AudioClock( bool Muted );
	void FrameClock( int cycles );

	bool FrameIRQ();

	void APUWrite( unsigned short Address, unsigned char Byte );
	unsigned char APURead( unsigned short Address, unsigned char Trash );

	/* Virtualize all the audio streams, for cartridge audio mixing */
	virtual float StreamAudio();

	/* EXPECTED OVERRIDES */
	virtual void FireSpriteDMA( unsigned char Address ) {}
private:
	float SquareVolume[31];
	float OtherVolume[203];

	bool m_Halt;
	bool m_FrameIRQ;

	int	 m_FrameTarget;
	int	 m_FrameError;

	int	 m_SequenceLength;
	bool m_InhibitIRQ;
	int  m_FrameCount;

	float m_OutputVolume;
	int m_OutputSamples;

	SquareAPU	Square1;
	SquareAPU	Square2;
	TriangleAPU	Triangle;
	NoiseAPU	Noise;

	Controller *CtrlPort1;
	Controller *CtrlPort2;
};

#endif