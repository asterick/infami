#include "Controller.h"

unsigned char Controller::ReadPort( unsigned char Trash ) 
{ 
	return Trash; 
};

void Controller::Strobe( unsigned char Byte )
{
}

void Controller::LightSense( bool pressed, bool Luma )
{
}

void Controller::HandleEvent( bool Pressed, unsigned int Code )
{
}
