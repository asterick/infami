#include "../Emulation/Controller.h"
#include "Gamepad.h"

#include "SDL.h"

Gamepad::Gamepad( GamepadConfig *config )
{
	Config = config;

	for( int i = 0; i < 8; i++ )
		keys[i] = false;
	code = 0;
}

unsigned char Gamepad::ReadPort( unsigned char Trash )
{
	if(	code < 8 && keys[code++] )
		return (Trash & 0xFE) | 0x01;

	return Trash & 0xFE;
}

void Gamepad::Strobe( unsigned char Byte )
{
	if( Byte & 1 )
		code = 0;
}

void Gamepad::HandleEvent( bool Pressed, unsigned int Code )
{	
	if( Code == Config->AKeyCode )
		keys[0] = Pressed;
	else if( Code == Config->BKeyCode )
		keys[1] = Pressed;
	else if( Code == Config->SelectKeyCode )
		keys[2] = Pressed;
	else if( Code == Config->StartKeyCode )
		keys[3] = Pressed;
	else if( Code == Config->UpKeyCode )
		keys[4] = Pressed;
	else if( Code == Config->DownKeyCode )
		keys[5] = Pressed;
	else if( Code == Config->LeftKeyCode )
		keys[6] = Pressed;
	else if( Code == Config->RightKeyCode )
		keys[7] = Pressed;
}