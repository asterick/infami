#undef UNICODE

#ifdef WIN32
	#include <windows.h>
	#include <direct.h>
#else
	#include <sys/stat.h>

	#define strcpy_s( a, s, b ) strcpy( a, b )
	#define strcat_s( a, s, b ) strcat( a, b )
#endif

#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "System.h"

char g_SavePath[CONFIG_STR_LEN];

void LoadConfiguration( const char* executable, SystemConfig& config )
{
	
	// -- Set save path inside of the install directory
	strcpy_s( g_SavePath, sizeof(g_SavePath), executable );
	char *ptr = g_SavePath + strlen(g_SavePath);
	while( *ptr !='\\' && *ptr != '/' && ptr != g_SavePath)
		ptr--;
	*ptr = 0;
	strcat_s( g_SavePath, sizeof(g_SavePath), "/saves" );
	
	#ifdef WIN32
		_mkdir( g_SavePath );
	#else
		mkdir( g_SavePath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
	#endif

	// --- FILL IN DEFAULT PARAMETERS BEFORE LOADING ---
	config.SampleRate = 44100;
	
	config.Gamepad1.AKeyCode = SDLK_x;
	config.Gamepad1.BKeyCode = SDLK_z;
	config.Gamepad1.SelectKeyCode = SDLK_TAB;
	config.Gamepad1.StartKeyCode = SDLK_RETURN;
	config.Gamepad1.UpKeyCode = SDLK_UP;
	config.Gamepad1.DownKeyCode = SDLK_DOWN;
	config.Gamepad1.LeftKeyCode = SDLK_LEFT;
	config.Gamepad1.RightKeyCode = SDLK_RIGHT;

	config.Gamepad2.AKeyCode = SDLK_k;
	config.Gamepad2.BKeyCode = SDLK_j;
	config.Gamepad2.SelectKeyCode = SDLK_g;
	config.Gamepad2.StartKeyCode = SDLK_h;
	config.Gamepad2.UpKeyCode = SDLK_w;
	config.Gamepad2.DownKeyCode = SDLK_s;
	config.Gamepad2.LeftKeyCode = SDLK_a;
	config.Gamepad2.RightKeyCode = SDLK_d;

	// --- Load previous configuration ---
 	char path[512];

    GetFileName( path, sizeof(path), executable, ".cfg" );
	SDL_RWops *fo = SDL_RWFromFile( path, "rb" );

	if( fo == NULL )
		return ;
	
	SDL_RWseek( fo, 0, SEEK_END );
	int length = SDL_RWtell(fo) + 1;
	SDL_RWseek( fo, 0, SEEK_SET );
	SDL_RWread( fo, &config, length, 1 );
	SDL_RWclose(fo);
}

void SaveConfiguration( const char* executable, SystemConfig& config )
{
	char path[512];
	
    GetFileName( path, sizeof(path), executable, ".cfg" );

	SDL_RWops *fo = SDL_RWFromFile( path, "wb" );

	if( fo == NULL )
		return ;

	SDL_RWwrite(fo, &config, sizeof(config), 1 );
	SDL_RWclose(fo);
}

#ifdef WIN32
bool GetRomFilename( char *fileName, int bufferLen )
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = fileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = bufferLen;
	ofn.lpstrFilter = "iNES Roms\0*.NES\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	if ( GetOpenFileName(&ofn) == TRUE )
		return true;
	else
		return false;
}

bool GetDiskFilename( char *fileName, int bufferLen )
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = fileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = bufferLen;
	ofn.lpstrFilter = "Famicom Disk System Image (*.FDS)\0*.FDS\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	if ( GetOpenFileName(&ofn) == TRUE )
		return true;
	else
		return false;
}
#else
bool GetRomFilename( char *fileName, int bufferLen )
{
    *fileName = 0;
    return false;
}

bool GetDiskFilename( char *fileName, int bufferLen )
{
    *fileName = 0;
    return false;
}
#endif

const char* GetDatabaseName()
{
	static char path[512];
	
    strcpy_s( path, sizeof(path), g_SavePath );
	strcat_s( path, sizeof(path), "/NstDatabase.dat" );
	
	return path;
}

bool GetFileName( char* dst, int size, const char *src, const char *extension )
{    
	const char *basePath = src + strlen(src);

    while( basePath != src && *basePath != '/' && *basePath != '\\' )
        basePath--;

    strcpy_s( dst, size, g_SavePath );
    strcat_s( dst, size, basePath );
    strcat_s( dst, size, extension );

	return true;
}

