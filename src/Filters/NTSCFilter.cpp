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

void NTSCFilter::BlitFrame( SDL_Surface *source, unsigned short *pixels, int PPU_Pitch )
{
	SDL_LockSurface( source );

	unsigned int *out_line = (unsigned int*) source->pixels;

	out_line += (source->w - NES_NTSC_OUT_WIDTH(PPU_Pitch)) / 2;

	nes_ntsc_blit( ntsc, pixels, PPU_Pitch, BurstPhase,
		PPU_Pitch, 240, out_line, source->pitch*2 );

	unsigned int *out_line_e = (unsigned int*) source->pixels;
	unsigned int *out_line_o = (unsigned int*) out_line_e + (source->pitch/4);

	for( int y = source->h/2; y; y-- )
	{
		for( int i = source->w; i; i-- )
			*(out_line_o++) = *(out_line_e++);

		out_line_o += (source->pitch/4);
		out_line_e += (source->pitch/4);
	}

	SDL_UnlockSurface(source);

	BurstPhase = 1-BurstPhase;
}