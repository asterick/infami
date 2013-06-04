#define _CRT_SECURE_NO_WARNINGS

#include "SDL.h"

#include "Emulation.h"
#include "../System.h"


ROM::ROM( const TimingSettings* timing ) :
	Famicom( timing )
{
	PRGRom = 0;
	CHRRom = 0;
	Trainer = 0;
	BCKRam = 0;	
	WRKRam = 0;

	BCKSize = 0;
	WRKSize = 0;
	BatteryBackup = false;

	memset( CHRRam, 0, sizeof(CHRRam) );
}

ROM::~ROM() 
{
	if( BatteryBackup )
	{
		SDL_RWops *fo = SDL_RWFromFile( FileName, "wb" );

		if( fo )
		{
			SDL_RWwrite( fo, BCKRam, BCKSize, 1 );
			SDL_RWclose(fo);
		}
	}

	if( BCKRam )
		delete BCKRam;
	if( PRGRom )
		delete PRGRom;
}

void ROM::Initalize( int Mirroring )
{
	switch( Mirroring )
	{
	case Vertical:
		SetMirrorVertical();
		break;
	case Horizontal:
		SetMirrorHorizontal();
		break;
	case FourScreen:
		SetMirrorFourScreen();
		break;
	case ZeroPage:
		SetMirrorZero();
		break ;
	case OnePage:
		SetMirrorOne();
		break ;
	default:
		// DEFAULT TO VERTICAL MIRRORING
		SetMirrorVertical();
		break ;
	}

	PrgMask = (PRGSize - 1);
	RamMask = (RAMSize - 1);
}

void ROM::LoadRam()
{
	if( BatteryBackup )
	{
		SDL_RWops *fo = SDL_RWFromFile( FileName, "rb" );

		if( fo )
		{
			SDL_RWread( fo, WRKRam, BatteryBackup, 1 );
			SDL_RWclose(fo);
		}
	}
}

void ROM::SetFileName( const char *fileName )
{
	GetFileName( FileName, sizeof(FileName), fileName, ".sav" );
}

Famicom *ROM::GetSystem() 
{ 
	return NULL; 
}
