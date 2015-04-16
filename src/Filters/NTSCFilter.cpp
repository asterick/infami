#include "SDL.h"

#include "VideoFilter.h"
#include "NTSCFilter.h"


NTSCFilter::NTSCFilter()
{
	ntsc = new nes_ntsc_t;
	nes_ntsc_init( ntsc, &nes_ntsc_composite );

	BurstPhase = 0;
}

NTSCFilter::~NTSCFilter()
{
	delete ntsc;
}

void NTSCFilter::BlitFrame( unsigned int *out_line, unsigned short *pixels, int PPU_Pitch )
{
	out_line += (640 - NES_NTSC_OUT_WIDTH(PPU_Pitch)) / 2;

	nes_ntsc_blit( ntsc, pixels, PPU_Pitch, BurstPhase,
		PPU_Pitch, 240, out_line, (640*sizeof(unsigned int))*2 );

	BurstPhase = 1-BurstPhase;
}