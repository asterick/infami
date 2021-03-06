#ifndef __NTSCFILTER_H
#define __NTSCFILTER_H

#include "nes_ntsc.h"

class NTSCFilter : public VideoFilter
{
public:
	NTSCFilter();
	~NTSCFilter();
	virtual void BlitFrame( SDL_Surface *source, unsigned short *pixels, int PPU_Pitch );

private:
	nes_ntsc_t* ntsc;
	SDL_Surface* Frame;
	int BurstPhase;
};

#endif