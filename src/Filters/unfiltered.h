#ifndef __UNFILTERED_H
#define __UNFILTERED_H

class Unfiltered : public VideoFilter
{
public:
	virtual void BlitFrame( unsigned int *source, unsigned short *pixels, int PPU_Pitch );
};

#endif
