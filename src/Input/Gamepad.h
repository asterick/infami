#ifndef __GAMEPAD_H
#define __GAMEPAD_H

struct GamepadConfig
{
	unsigned int	UpKeyCode;
	unsigned int	DownKeyCode;
	unsigned int	LeftKeyCode;
	unsigned int	RightKeyCode;
	unsigned int	StartKeyCode;
	unsigned int	SelectKeyCode;
	unsigned int	BKeyCode;
	unsigned int	AKeyCode;
};

class Gamepad : public Controller
{
public:
	Gamepad( GamepadConfig *config );
	unsigned char ReadPort( unsigned char Trash );
	void Strobe( unsigned char Byte );
	void HandleEvent( bool Pressed, unsigned int Code );
private:
	GamepadConfig *Config;

	bool keys[8];
	int code;
};

#endif