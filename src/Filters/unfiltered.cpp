#include "../Emulation/PPU.h"

#include "VideoFilter.h"
#include "unfiltered.h"

int k;

void Unfiltered::BlitFrame( uint32_t *source, int pitch, uint16_t *pixels )
{
	for (int y = 0; y < PPU_HEIGHT; y++) {
		for (int x = 0; x < PPU_WIDTH; x++) {
			*(source++) = 0xFF000000 | (x + k++); //NesPalette[*pixels & 0x1FF];
		}

		source += SCREEN_WIDTH - PPU_WIDTH;
	}
}
