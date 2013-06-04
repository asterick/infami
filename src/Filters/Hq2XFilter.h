#ifndef __HQ2X_H
#define __HQ2X_H

class Hq2XFilter : public VideoFilter
{
public:
	Hq2XFilter();
	virtual void BlitFrame( SDL_Surface *source, unsigned short *pixels, int PPU_Pitch );
	void hq2x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int BpL );
};

#endif