#include "SDL.h"

#include "VideoFilter.h"
#include "unfiltered.h"

void Unfiltered::BlitFrame( SDL_Surface *source, unsigned short *pixels, int PPU_Pitch )
{
	SDL_LockSurface( source );

	int	in_row_width				= PPU_Pitch;
	int out_pitch					= source->pitch/2;

	unsigned short const* in_line	= pixels;
	unsigned int* out_line			= (unsigned int*)source->pixels;
	unsigned int* out_line_o		= out_line + (out_pitch / 2);

	out_line += (source->w - PPU_Pitch*2) / 2;
	out_line_o += (source->w - PPU_Pitch*2) / 2;

	for( int y = source->h / 2; y; y-- )
	{
		unsigned short const* in_pixel	= in_line;
		unsigned int* out_pixel			= out_line;
		unsigned int* out_pixel_o		= out_line_o;

		in_line += in_row_width;
		out_line += out_pitch;
		out_line_o += out_pitch;

		for( int x = PPU_Pitch; x; x-- )
		{
			unsigned int clr = NesPalette[ *(in_pixel++) & 0x1FF ];
			*(out_pixel++) = clr;
			*(out_pixel++) = clr;

			clr  = (clr >> 1) & 0x7F7F7F7F;
			clr += (clr >> 1) & 0x7F7F7F7F;

			*(out_pixel_o++) = clr;
			*(out_pixel_o++) = clr;
		}
	}

	SDL_UnlockSurface(source);
}
