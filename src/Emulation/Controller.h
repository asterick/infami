#ifndef __CONTROLLER_H
#define __CONTROLLER_H

class Controller
{
public:
	virtual unsigned char ReadPort( unsigned char Trash );
	virtual void Strobe( unsigned char Byte );
	virtual void LightSense( bool pressed, bool Luma );
	virtual void HandleEvent( bool Pressed, unsigned int Code );
};

#endif