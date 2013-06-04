#ifndef __INES_H
#define __INES_H

struct iNES_Header
{
	char Identifier[4];
	unsigned char PRG_Pages;
	unsigned char CHR_Pages;
	unsigned char Flags6;
	unsigned char Flags7;
	unsigned char PRG_Ram;
	unsigned char Flags9;
	unsigned char Flags10;

	unsigned char Zero[5];
};

void LoadDatabase( const char *path );
void CloseDatabase();

template <class T> ROM* ConstructMapper( const TimingSettings *Timing ) { return new T(Timing); }
typedef ROM* (*MapperConstructor)( const TimingSettings *Timing );
Famicom* LoadINes( char *path );

#endif