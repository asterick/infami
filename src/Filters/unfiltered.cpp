#include "VideoFilter.h"
#include "unfiltered.h"

void Unfiltered::BlitFrame( unsigned int *source, unsigned short *pixels, int PPU_Pitch )
{
	int	in_row_width				= PPU_Pitch;
	int out_pitch					= SCREEN_PITCH/2;

	unsigned short const* in_line	= pixels;
	unsigned int* out_line			= (unsigned int*)source;
	unsigned int* out_line_o		= out_line + (out_pitch / 2);

	out_line += (SCREEN_WIDTH - PPU_Pitch*2) / 2;
	out_line_o += (SCREEN_WIDTH - PPU_Pitch*2) / 2;

	for( int y = SCREEN_HEIGHT / 2; y; y-- )
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
}
