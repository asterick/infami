#ifndef __SYSTEM_H
#define __SYSTEM_H

/***
 ***	NES TIMING DIFFERENCES
 ***/

class Famicom : public APU, public PPU, public R6502
{
public:
	Famicom( const TimingSettings* timing );
	virtual ~Famicom();

	unsigned short* Execute();
	virtual void SpecialKey( int Code ) {}

	/* Special Audio Buffering Calls */
	virtual void ConfigureAudio( int SampleRate, int BufferLength );
	virtual float *AudioStream();
	virtual bool AudioFull();
	virtual void MuteAudio( bool pause );
protected:
	bool HaltActive();
	void FireSpriteDMA( unsigned char Address );
	unsigned char DMCBusRead( unsigned short addr );

	virtual void CPUClock() {}
	virtual bool NMIActive();
	virtual bool IRQActive();
	void Clock();

	unsigned char SystemRam[0x800];
private:
	int m_CPUClockDivide;
	int m_PPUClockDivide;
	int m_CPUError;

	/* AUDIO BUFFERING */
	// Cummulative error code
	int	m_InputRate;
	int m_OutputRate;
	int m_OutputDivide;
	int m_OutputError;	

	bool m_BufferWaiting;
	bool m_MutedAudio;

	int m_BufferLength;
	int m_SamplesLeft;
	float *m_Buffer;
	float *m_BufferActive;
	float *m_BufferWrite;
	float *m_BackBuffer;
	float *m_AudioBuffers;
	
	unsigned short *m_Frame;
};

#endif