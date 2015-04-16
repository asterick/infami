#ifndef __NTSCFILTER_H
#define __NTSCFILTER_H

#include "nes_ntsc.h"

class NTSCFilter : public VideoFilter
{
public:
	NTSCFilter();
	~NTSCFilter();
	virtual void BlitFrame( uint32_t *screen, int pitch, uint16_t *pixels );

private:
	nes_ntsc_t* ntsc;
	int BurstPhase;
};

#endif
