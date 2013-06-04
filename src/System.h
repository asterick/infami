#ifndef __GUISTUFF_H
#define __GUISTUFF_H

#define CONFIG_STR_LEN	(512)

#include "Emulation/Controller.h"
#include "Input/Gamepad.h"

struct SystemConfig
{
	int				SampleRate;
	GamepadConfig	Gamepad1;
	GamepadConfig	Gamepad2;
};

extern "C" 
{
	bool GetRomFilename( char *fileName, int bufferLen );
	bool GetDiskFilename( char *fileName, int bufferLen );
	bool GetFileName( char* dst, int size, const char *src, const char *extension );
	const char* GetDatabaseName();
	
	void LoadConfiguration( const char* executable, SystemConfig& config );
	void SaveConfiguration( const char* executable, SystemConfig& config );
}

#endif