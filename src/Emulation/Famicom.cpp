#include "Emulation.h"

Famicom::Famicom( const TimingSettings* timing )
	: APU( timing ), PPU( timing->BlankingLines ), R6502()
{
	m_CPUClockDivide = timing->CPUDivider;
	m_PPUClockDivide = timing->PPUDivider;

	// Setup base timing
	m_InputRate	 = timing->SystemClock;
	m_OutputDivide = timing->CPUDivider;

	m_CPUError = 0;

	m_AudioBuffers = NULL;	
	m_Frame = NULL;

	// Zero out system ram
	memset( SystemRam, 0, sizeof( SystemRam ) );
}

Famicom::~Famicom()
{
	if( m_AudioBuffers )
		delete m_AudioBuffers;
}

unsigned short *Famicom::Execute()
{
	R6502::CPUStep();

	unsigned short *frame = m_Frame;
	m_Frame = NULL;
	return frame;
}

void Famicom::Clock()
{	
	// Clock the chips	
	APU::AudioClock( m_MutedAudio );
	CPUClock();

	m_CPUError += m_CPUClockDivide;
	while( m_CPUError >= m_PPUClockDivide )
	{
		unsigned short *frame = PPU::VideoClock();
		APU::FrameClock( m_PPUClockDivide );

		if( frame != NULL )
			m_Frame = frame;

		m_CPUError -= m_PPUClockDivide;
	}

	// Buffer audio	
	if( !m_MutedAudio && (m_OutputError += m_OutputRate) >= m_InputRate )
	{
		*(m_Buffer++) = StreamAudio();

		if( --m_SamplesLeft == 0 )
		{
			m_Buffer = m_BackBuffer;
			m_BackBuffer = m_BufferActive;
			m_BufferActive = m_BufferWrite;
			m_BufferWrite = m_Buffer;

			m_SamplesLeft = m_BufferLength;
			m_BufferWaiting = true;
		}

		m_OutputError -= m_InputRate;
	}
}

/* --- AUDIO BUFFERING CALLS --- */

void Famicom::ConfigureAudio( int SampleRate, int BufferLength )
{
	// Calculate the APU sample rate
	m_OutputRate = SampleRate * m_OutputDivide;

	// Initalize our audio stream
	m_OutputError = 0;

	m_BufferLength = BufferLength;
	m_AudioBuffers = new float[BufferLength*3];
	m_SamplesLeft = BufferLength;

	m_Buffer		= m_BufferWrite = m_AudioBuffers;
	m_BufferActive	= &m_AudioBuffers[m_BufferLength];
	m_BackBuffer	= &m_AudioBuffers[m_BufferLength*2];

	m_MutedAudio = false;

	for( int i = 0; i < m_BufferLength*3; i++ )
		m_AudioBuffers[i] = 0;
}


void Famicom::MuteAudio( bool pause )
{
	m_MutedAudio = pause;
}

bool Famicom::AudioFull()
{
	return m_BufferWaiting && !m_MutedAudio;
}

float *Famicom::AudioStream()
{
	m_BufferWaiting = false;
	return m_BufferActive;
}

/* --- CHIP GLUE --- */

bool Famicom::IRQActive()
{
	return APU::FrameIRQ();
}

bool Famicom::NMIActive()
{
	return PPU::RequestNMI();
}

bool Famicom::HaltActive()
{
	return DeltaModAPU::RequestHalt();
}

void Famicom::FireSpriteDMA( unsigned char Address )
{
	R6502::SpriteDMA(Address);
}

unsigned char Famicom::DMCBusRead( unsigned short addr )
{	
	return Read( addr );
}

