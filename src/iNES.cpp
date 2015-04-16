#include "SDL2/SDL.h"

#include "Emulation/Emulation.h"
#include "Mappers/Mappers.h"

#include "iNES.h"
#include "Mappers/MapperTable.h"
#include "headerDatabase.h"

unsigned int crc32( const unsigned char *buf, unsigned int len )
{
	if (buf == NULL)
		return 0L;

	unsigned int crc = 0xffffffffL;
	while (len--)
		crc = crc_table[(crc ^ *(buf++)) & 0xff] ^ (crc >> 8);
	return crc ^ 0xffffffffL;
}

void LoadDatabase( const char *path )
{
	SDL_RWops *fo = SDL_RWFromFile(path, "rb");

	if( fo == NULL )
	{
		printf("Unable to locate database: %s\n", path);
		nestopiaDB = new RomDescription[ 1 ];
		nestopiaDB[0].CRC32 = 0;
		return ;
	}

	int files;

	SDL_RWread( fo, &files, 4, 1 );

	files = SDL_SwapLE32(files);

	nestopiaDB = new RomDescription[ files + 1 ];
	for( int i = 0; i < files; i++ )
	{
		SDL_RWread( fo, &nestopiaDB[i], 14, 1 );
		nestopiaDB[i].CRC32 = SDL_SwapLE32(nestopiaDB[i].CRC32);
		nestopiaDB[i].flags = SDL_SwapLE16(nestopiaDB[i].flags);
	}

	nestopiaDB[files].CRC32 = 0;

	printf("Nestopia header database found... %i decriptions loaded.\n", files );

	SDL_RWclose(fo);
}

void CloseDatabase()
{
	delete nestopiaDB;
}

Famicom* LoadINes( const char *path )
{
	SDL_RWops *fo = SDL_RWFromFile(path, "rb");

	if( fo == NULL )
	{
		printf("File %s not found\n", path);
		return NULL;
	}

	iNES_Header header;

	SDL_RWseek( fo, 0, SEEK_END );
	int romLength = SDL_RWtell(fo) - 16;
	SDL_RWseek( fo, 0, SEEK_SET );

	unsigned char *romData = new unsigned char[romLength];
	SDL_RWread( fo, &header, sizeof(header), 1 );
	SDL_RWread( fo, romData, romLength, 1 );
	SDL_RWclose(fo);

	// Verify this is an iNES file
	if( header.Identifier[0] != 'N' ||
		header.Identifier[1] != 'E' ||
		header.Identifier[2] != 'S' ||
		header.Identifier[3] != 0x1A )
	{
		printf( "Not a valid .NES file\n" );
		return NULL;
	}

	unsigned int crc = crc32( romData, romLength );

	const RomDescription *romDesc = nestopiaDB;
	ROM *rom;
	int Mirroring;

	while( romDesc->CRC32 && romDesc->CRC32 != crc )
		romDesc++;

	if( romDesc->CRC32 == 0 )
	{
		printf("WARNING: This rom was not recognized in the header database\n");
		printf("Assuming use of an Extended iNES header.\n");

		for( int i = 0; i < 5; i++ )
		{
			if( header.Zero[i] != 0 )
			{
				printf("Header is corrupt.  This rom may not work.\n");
				header.Flags7 = 0;
				header.PRG_Ram = 1;
				header.Flags9 = 0;
				header.Flags10 = 0;

				break ;
			}
		}

		// Maintain a minimum of one page of WRam
		if( header.PRG_Ram < 1 )
			header.PRG_Ram = 1;

		int mapper = (header.Flags7 & 0xF0) | (header.Flags6 >> 4);
		printf("Loading mapper %i\n", mapper );

		// Ignore trainers
		if( header.Flags6 & 0x04 )
			romData += 512;

		if( LoadMapper[ mapper ] == NULL )
		{
			printf( "Unrecognized mapper %i\n", mapper ) ;
			return NULL;
		}

		rom = LoadMapper[ mapper ]( (header.Flags10 & 0x0002) ? &PALTiming : &NTSCTiming );

		rom->PRGRom		= romData;
		rom->PRGSize	= header.PRG_Pages * 0x4000;

		if( header.CHR_Pages == 0 )
		{
			rom->CHRRom = rom->CHRRam;
			rom->CHRSize = 0x2000;
		}
		else
		{
			rom->CHRRom = rom->PRGRom + header.PRG_Pages * 0x4000;
			rom->CHRSize = header.CHR_Pages * 0x2000;
		}

		rom->WRKSize = header.PRG_Ram * 0x2000;
		rom->BCKSize = header.PRG_Ram * 0x2000;
		rom->RAMSize = rom->BCKSize + rom->WRKSize;

		rom->BCKRam = new unsigned char[rom->RAMSize];
		rom->WRKRam = rom->BCKRam + rom->BCKSize;

		rom->BatteryBackup = ( header.Flags6 & 0x02 ) ? true : false;

		switch( header.Flags6 & 9 )
		{
		case 0:
			Mirroring = Horizontal;
			break ;
		case 1:
			Mirroring = Vertical;
			break ;
		default:
			Mirroring = FourScreen;
			break ;
		}

		// TODO: HANDLE VS
		// TODO: HANDLE P10
		// TODO: HANDLE BUS CONFLICTS
	}
	else
	{
		// --- USE NESTOPIA DATABASE RATHER THAN THE PRE-CREATED iNES HEADER! ---

		printf("Loading mapper %i\n", romDesc->mapper );
		if( LoadMapper[ romDesc->mapper ] == NULL )
		{
			printf( "Unrecognized mapper %i\n", romDesc->mapper ) ;
			return NULL;
		}

		// Ignore trainers
		if( romDesc->flags & 0x0100 )
			romData += 512;

		rom = LoadMapper[ romDesc->mapper ]( (romDesc->flags & 0x0002) ? &NTSCTiming : &PALTiming );
		rom->Trainer = romData;

		rom->PRGRom		= romData;
		rom->PRGSize	= romDesc->prgSize * 0x4000;

		if( romDesc->chrSize == 0 )
		{
			rom->CHRRom = rom->CHRRam;
			rom->CHRSize = 0x2000;
		}
		else
		{
			rom->CHRRom = rom->PRGRom + (romDesc->prgSize + romDesc->prgSkip) * 0x4000;
			rom->CHRSize = romDesc->chrSize * 0x2000;
		}

		if( romDesc->wrkSize > 0 )
		{
			if( (romDesc->wrkSize & 0x0F) != 0 )
				rom->BCKSize	= 64 << (romDesc->wrkSize & 0x0F);
			if( (romDesc->wrkSize & 0xF0) != 0 )
				rom->WRKSize	= 64 << ((romDesc->wrkSize & 0xF0)>>4);

			rom->RAMSize = rom->WRKSize + rom->BCKSize;

			rom->BCKRam = new unsigned char[rom->RAMSize];
			rom->WRKRam = rom->BCKRam + rom->BCKSize;
		}

		/*
		FLAGS_PAL       = 0x0001,	//
		FLAGS_NTSC      = 0x0002,	//
		FLAGS_VS        = 0x0004,
		FLAGS_P10       = 0x0008,
		FLAGS_MIRRORING = 0x0070,	//
		FLAGS_TRAINER   = 0x0100,	//
		FLAGS_BAD       = 0x0200,
		FLAGS_PRG_HI    = 0x0400,
		FLAGS_ENCRYPTED = 0x0800,

		FLAGS_MIRRORING_SHIFT = 4,
		FLAGS_PRG_HI_SHIFT = 2,

		INPUT_BITS = 0x1F,
		INPUT_EX_SHIFT = 5
		*/

		// TODO: HANDLE INPUT
		// TODO: HANDLE VS
		// TODO: HANDLE P10
		// TODO: HANDLE BUS CONFLICTS

		switch( romDesc->flags & 0x70 )
		{
		case 0x00:
			Mirroring	= Horizontal;
			break ;
		case 0x10:
			Mirroring  = Vertical;
			break ;
		case 0x20:
			Mirroring	= FourScreen;
			break ;
		case 0x30:
			Mirroring	= ZeroPage;
			break ;
		case 0x40:
			Mirroring	= OnePage;
			break ;
		default:
			Mirroring	= MapperControlled;
			break ;
		}

		// ----------------------------------------------------------------------
	}

	rom->SetFileName( path );
	rom->LoadRam();
	rom->Initalize( Mirroring );

	return rom->GetSystem();
}
