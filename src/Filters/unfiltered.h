#ifndef __UNFILTERED_H
#define __UNFILTERED_H

class Unfiltered : public VideoFilter
{
public:
	virtual void BlitFrame( uint32_t *source, uint16_t *pixels, int PPU_Pitch );
};

#endif
