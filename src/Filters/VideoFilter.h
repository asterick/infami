#ifndef __VIDEOFILTER_H
#define __VIDEOFILTER_H

const unsigned int NesPalette[512] = 
{
	0x525252, 0x000080, 0x08008a, 0x2c007e, 0x4a004e, 0x500006, 0x440000, 0x260800, 
	0x0a2000, 0x002e00, 0x003200, 0x00260a, 0x001c48, 0x000000, 0x000000, 0x000000, 
	0xa4a4a4, 0x0038ce, 0x3416ec, 0x5e04dc, 0x8c00b0, 0x9a004c, 0x901800, 0x703600, 
	0x4c5400, 0x0e6c00, 0x007400, 0x006c2c, 0x005e84, 0x000000, 0x000000, 0x000000, 
	0xffffff, 0x4c9cff, 0x7c78ff, 0xa664ff, 0xda5aff, 0xf054c0, 0xf06a56, 0xd68610, 
	0xbaa400, 0x76c000, 0x46cc1a, 0x2ec866, 0x34c2be, 0x3a3a3a, 0x000000, 0x000000, 
	0xffffff, 0xb6daff, 0xc8caff, 0xdac2ff, 0xf0beff, 0xfcbcee, 0xfac2c0, 0xf2cca2, 
	0xe6da92, 0xcce68e, 0xb8eea2, 0xaeeabe, 0xaee8e2, 0xb0b0b0, 0x000000, 0x000000, 
	0x654b3c, 0x00005f, 0x090066, 0x36005d, 0x5b0039, 0x630004, 0x540000, 0x2f0700, 
	0x0c1d00, 0x002a00, 0x002d00, 0x002207, 0x001935, 0x000000, 0x000000, 0x000000, 
	0xcb9679, 0x003399, 0x4014af, 0x7403a3, 0xad0082, 0xbe0038, 0xb21500, 0x8a3100, 
	0x5e4c00, 0x116200, 0x006a00, 0x006220, 0x005662, 0x000000, 0x000000, 0x000000, 
	0xffe9bd, 0x5e8ebd, 0x996dbd, 0xcd5bbd, 0xff52bd, 0xff4c8e, 0xff603f, 0xff7a0b, 
	0xe69600, 0x92af00, 0x56ba13, 0x38b74b, 0x40b18d, 0x47352b, 0x000000, 0x000000, 
	0xffe9bd, 0xe1c7bd, 0xf7b8bd, 0xffb1bd, 0xffadbd, 0xffacb0, 0xffb18e, 0xffba78, 
	0xffc76c, 0xfcd269, 0xe3d978, 0xd7d68d, 0xd7d4a7, 0xdaa182, 0x000000, 0x000000, 
	0x415948, 0x000070, 0x060079, 0x22006f, 0x3a0044, 0x3f0005, 0x350000, 0x1e0800, 
	0x072200, 0x003100, 0x003600, 0x002908, 0x001e3f, 0x000000, 0x000000, 0x000000, 
	0x82b290, 0x003cb5, 0x2917d0, 0x4a04c2, 0x6f009b, 0x7a0043, 0x721a00, 0x583a00, 
	0x3c5b00, 0x0b7500, 0x007d00, 0x007526, 0x006674, 0x000000, 0x000000, 0x000000, 
	0xcaffe0, 0x3ca9e0, 0x6282e0, 0x836ce0, 0xad61e0, 0xbe5ba9, 0xbe734b, 0xa9910e, 
	0x93b200, 0x5dd000, 0x37dd16, 0x24d959, 0x29d2a7, 0x2e3e33, 0x000000, 0x000000, 
	0xcaffe0, 0x90ece0, 0x9edbe0, 0xadd2e0, 0xbecee0, 0xc8ccd1, 0xc6d2a9, 0xc0dd8e, 
	0xb6ec80, 0xa1f97d, 0x92ff8e, 0x8afea7, 0x8afbc7, 0x8bbf9b, 0x000000, 0x000000, 
	0x535035, 0x000053, 0x08005a, 0x2c0052, 0x4b0032, 0x510003, 0x450000, 0x260700, 
	0x0a1f00, 0x002d00, 0x003100, 0x002506, 0x001b2f, 0x000000, 0x000000, 0x000000, 
	0xa7a06b, 0x003686, 0x34159a, 0x5f038f, 0x8e0072, 0x9c0031, 0x921700, 0x723400, 
	0x4d5200, 0x0e6900, 0x007100, 0x00691c, 0x005c56, 0x000000, 0x000000, 0x000000, 
	0xfff9a6, 0x4d98a6, 0x7e75a6, 0xa962a6, 0xde58a6, 0xf4527d, 0xf46738, 0xda830a, 
	0xbda000, 0x78bc00, 0x47c710, 0x2ec442, 0x34be7c, 0x3b3825, 0x000000, 0x000000, 
	0xfff9a6, 0xb9d5a6, 0xcbc5a6, 0xdebea6, 0xf4baa6, 0xffb89b, 0xfebe7d, 0xf6c769, 
	0xead55f, 0xcfe15c, 0xbbe969, 0xb1e57c, 0xb1e393, 0xb3ac72, 0x000000, 0x000000, 
	0x4a5468, 0x0000a3, 0x0700b0, 0x2700a0, 0x420063, 0x480007, 0x3d0000, 0x220800, 
	0x092000, 0x002f00, 0x003300, 0x00260c, 0x001c5b, 0x000000, 0x000000, 0x000000, 
	0x94a8d1, 0x0039ff, 0x2f16ff, 0x5504ff, 0x7e00e0, 0x8b0061, 0x821800, 0x653700, 
	0x445600, 0x0c6e00, 0x007700, 0x006e38, 0x0060a8, 0x000000, 0x000000, 0x000000, 
	0xe6ffff, 0x44a0ff, 0x707bff, 0x9666ff, 0xc55cff, 0xd956f5, 0xd96c6d, 0xc18914, 
	0xa8a800, 0x6ac400, 0x3fd121, 0x29cd82, 0x2fc7f2, 0x343b4a, 0x000000, 0x000000, 
	0xe6ffff, 0xa4dfff, 0xb5cfff, 0xc5c7ff, 0xd9c2ff, 0xe4c0ff, 0xe2c7f5, 0xdbd1ce, 
	0xd0dfba, 0xb8ebb5, 0xa6f4ce, 0x9df0f2, 0x9deeff, 0x9fb4e0, 0x000000, 0x000000, 
	0x534a50, 0x00007d, 0x080087, 0x2d007b, 0x4b004c, 0x510005, 0x450000, 0x260700, 
	0x0a1d00, 0x002900, 0x002d00, 0x002209, 0x001946, 0x000000, 0x000000, 0x000000, 
	0xa794a0, 0x0032c9, 0x3513e7, 0x6003d7, 0x8f00ac, 0x9d004a, 0x931500, 0x723100, 
	0x4d4c00, 0x0e6200, 0x006900, 0x00622b, 0x005581, 0x000000, 0x000000, 0x000000, 
	0xffe7f9, 0x4d8df9, 0x7e6cf9, 0xa95af9, 0xdf51f9, 0xf54cbb, 0xf56054, 0xda790f, 
	0xbe9400, 0x78ae00, 0x47b919, 0x2fb563, 0x35b0ba, 0x3b3438, 0x000000, 0x000000, 
	0xffe7f9, 0xbac5f9, 0xccb7f9, 0xdfb0f9, 0xf5acf9, 0xffaae9, 0xffb0bb, 0xf7b99e, 
	0xebc58e, 0xd0d08b, 0xbcd89e, 0xb2d4ba, 0xb2d2dd, 0xb49fac, 0x000000, 0x000000, 
	0x3c5052, 0x000080, 0x05008a, 0x20007e, 0x36004e, 0x3b0006, 0x320000, 0x1c0700, 
	0x071f00, 0x002d00, 0x003100, 0x00250a, 0x001b48, 0x000000, 0x000000, 0x000000, 
	0x79a1a4, 0x0037ce, 0x2615ec, 0x4503dc, 0x6700b0, 0x72004c, 0x6a1700, 0x523500, 
	0x385200, 0x0a6a00, 0x007200, 0x006a2c, 0x005c84, 0x000000, 0x000000, 0x000000, 
	0xbcfbff, 0x3899ff, 0x5b76ff, 0x7b62ff, 0xa158ff, 0xb152c0, 0xb16856, 0x9e8410, 
	0x89a100, 0x57bd00, 0x33c91a, 0x22c566, 0x26bfbe, 0x2a393a, 0x000000, 0x000000, 
	0xbcfbff, 0x86d7ff, 0x94c7ff, 0xa1bfff, 0xb1bbff, 0xbab9ee, 0xb9bfc0, 0xb3c9a2, 
	0xaad792, 0x97e38e, 0x88eaa2, 0x80e6be, 0x80e4e2, 0x82adb0, 0x000000, 0x000000, 
	0x3d3d3d, 0x000060, 0x060067, 0x21005e, 0x37003a, 0x3c0004, 0x330000, 0x1c0600, 
	0x071800, 0x002200, 0x002500, 0x001c07, 0x001536, 0x000000, 0x000000, 0x000000, 
	0x7b7b7b, 0x002a9a, 0x2710b1, 0x4603a5, 0x690084, 0x730039, 0x6c1200, 0x542800, 
	0x393f00, 0x0a5100, 0x005700, 0x005121, 0x004663, 0x000000, 0x000000, 0x000000, 
	0xbfbfbf, 0x3975bf, 0x5d5abf, 0x7c4bbf, 0xa343bf, 0xb43f90, 0xb44f40, 0xa0640c, 
	0x8b7b00, 0x589000, 0x349913, 0x22964c, 0x27918e, 0x2b2b2b, 0x000000, 0x000000, 
	0xbfbfbf, 0x88a3bf, 0x9697bf, 0xa391bf, 0xb48ebf, 0xbd8db2, 0xbb9190, 0xb59979, 
	0xaca36d, 0x99ac6a, 0x8ab279, 0x82af8e, 0x82aea9, 0x848484, 0x000000, 0x000000
};


class VideoFilter
{
public:
	virtual void BlitFrame( SDL_Surface *source, unsigned short *pixels, int PPU_Pitch ) {}
};

#endif