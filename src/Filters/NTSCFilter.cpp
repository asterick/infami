#include "../Emulation/PPU.h"

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

void NTSCFilter::BlitFrame( uint32_t *out_line, int pitch, uint16_t *pixels )
{
	out_line += (SCREEN_WIDTH - NES_NTSC_OUT_WIDTH(PPU_WIDTH)) / 2;

	nes_ntsc_blit( ntsc, pixels, PPU_WIDTH, BurstPhase,
		PPU_WIDTH, 240, out_line, pitch*2 );

	BurstPhase = 1-BurstPhase;
}
