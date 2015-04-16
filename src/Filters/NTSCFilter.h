#ifndef __NTSCFILTER_H
#define __NTSCFILTER_H

#include "nes_ntsc.h"

class NTSCFilter : public VideoFilter
{
public:
	NTSCFilter();
	~NTSCFilter();
	virtual void BlitFrame( uint32_t *screen, uint16_t *pixels, int PPU_Pitch );

private:
	nes_ntsc_t* ntsc;
	int BurstPhase;
};

#endif
