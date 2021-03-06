#include "SDL.h"
#include "System.h"

#include "Emulation/Emulation.h"
#include "Input/Gamepad.h"
#include "iNES.h"

#include "Filters/VideoFilter.h"
#include "Filters/NTSCFilter.h"
#include "Filters/Hq2XFilter.h"
#include "Filters/Unfiltered.h"

Gamepad *ControlPort1 = NULL;
Gamepad *ControlPort2 = NULL;
Famicom *machine = NULL;
SystemConfig config;

void ShowFPS()
{
	static Uint32 lastframe;
	static int ff=0;
	Uint32 thisframe = SDL_GetTicks();

	ff++;

	if((thisframe-lastframe) > 1000) 
	{
		printf("FPS: %d\n",ff);
		ff=0;
		lastframe=thisframe;
	}
}

void AudioCallback(void *sys, Uint8 *stream, int len)
{
	short *buffer = (short *)stream;
	float *wave   = ((Famicom *)sys)->AudioStream();

	while( len )
	{
		*(buffer++) = (short)(*(wave++) * 0x7000);
		len -= sizeof(short);
	}
}

void ConfigureAudio( int rate, Famicom *sys )
{
	SDL_AudioSpec desired, obtained;

	desired.freq=rate;
	desired.format=AUDIO_S16;
	desired.channels=1;
	desired.samples=2048;
	desired.callback=AudioCallback;
	desired.userdata=sys;

	/* Open the audio device */
	if ( SDL_OpenAudio(&desired, &obtained) < 0 ){
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(-1);
	}

	SDL_PauseAudio( false );

	sys->ConfigureAudio( obtained.freq, obtained.samples );
}

void loadFamicom( char *szFile )
{
	Famicom *loaded;
	loaded = LoadINes( szFile );
	
	if( loaded )
	{
		// Reset the NES (grrrr)
		loaded->Restart();
		
		if( machine != NULL )
		{
			SDL_CloseAudio();
			delete machine;
		}
		
		ConfigureAudio(config.SampleRate, loaded);
		
		if( ControlPort1 )
			loaded->InsertController( false, ControlPort1 );
		if( ControlPort2 )
			loaded->InsertController( true, ControlPort2 );
		machine = loaded;
	}
}

int main( int argc, char** argv )
{
	SDL_Surface *screen = NULL;
		
	LoadConfiguration( argv[0], config );
	
	/* initialize SDL */
	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK ) < 0 )
	{
		fprintf( stderr, "SDL initialization failed: %s\n",
			SDL_GetError( ) );

		SDL_Quit( );
		return -1;
	}

	if ( SDL_Init( SDL_INIT_JOYSTICK ) < 0 )
	{
		SDL_Quit();
		return -1;
	}

	int videoModes = SDL_SWSURFACE;

	screen = SDL_SetVideoMode( 640, 480, 32, videoModes );
	SDL_WM_SetCaption( "inFami", NULL );

	for( int i = 0; i < SDL_NumJoysticks(); i++ )
		SDL_JoystickOpen(i);

	LoadDatabase(GetDatabaseName());

	VideoFilter *filter = new NTSCFilter();
	ControlPort1 = new Gamepad( &config.Gamepad1 );
	ControlPort2 = new Gamepad( &config.Gamepad2 );

	if( argc > 1 )
		loadFamicom( argv[1] );

	// --- CORE EMULATION / PROGRAM LOOP ---
		
	bool running = true;

	do
	{
		SDL_Event event;

		while(SDL_PollEvent(&event)) 
		{
			switch(event.type) 
			{
				case SDL_QUIT:
					running = false;
					break ;
				case SDL_KEYDOWN:  /* Handle a KEYDOWN event */
					// Meta is used so mac keys make sense
					if( event.key.keysym.mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_META) )	
					{
						switch( event.key.keysym.sym )
						{
						case SDLK_f:
							videoModes ^= SDL_FULLSCREEN;
							screen = SDL_SetVideoMode( 640, 480, 32, videoModes );
							SDL_WM_SetCaption( "inFami", NULL );
							SDL_ShowCursor( (videoModes & SDL_FULLSCREEN) ? SDL_DISABLE : SDL_ENABLE );

							break ;
						case SDLK_q:
							running = false;
							break ;					
						case SDLK_o:
							SDL_PauseAudio(true);
							
							char szFile[1024];			

							if ( GetRomFilename( szFile, sizeof(szFile) ) )
								loadFamicom( szFile );

							SDL_PauseAudio( false );

							break ;
						case SDLK_r:
							if( machine )
								machine->Restart();
							break ;
						// THESE ARE MAPPER SHIFTED CODES
						default:
							if( machine )
								machine->SpecialKey( event.key.keysym.sym );
							break ;
						}
					}
				case SDL_KEYUP:
					if( event.key.keysym.mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_META) )
						break ;

					if( ControlPort1 != NULL )
						ControlPort1->HandleEvent( event.key.state == SDL_PRESSED, event.key.keysym.sym );
					if( ControlPort2 != NULL )
						ControlPort2->HandleEvent( event.key.state == SDL_PRESSED, event.key.keysym.sym );

					break ;
				case SDL_JOYAXISMOTION:
					{
						int buttonID = 0x81000000 | 
							(event.jhat.which) | 
							(event.jhat.hat << 8) | 
							((event.jhat.value > 0) ? 0x01 : 0x02);
						bool pressed = (abs(event.jaxis.value) > 0xFF);

						if( ControlPort1 != NULL )
							ControlPort1->HandleEvent( pressed, buttonID );
						if( ControlPort2 != NULL )
							ControlPort2->HandleEvent( pressed, buttonID );
					}
					break;

				case SDL_JOYHATMOTION:
					{
						int buttonID = 0x82000000 | 
							(event.jhat.which) | 
							(event.jhat.hat << 8) | 
							((event.jhat.value > 0) ? 0x01 : 0x02);

						bool pressed = (abs(event.jhat.value) > 0x40);

						if( ControlPort1 != NULL )
							ControlPort1->HandleEvent( pressed, buttonID );
						if( ControlPort2 != NULL )
							ControlPort2->HandleEvent( pressed, buttonID );
					}
					break ;
	
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
					{
						int buttonID = 0x83000000 |
							(event.jbutton.which) | 
							(event.jbutton.button << 8);
						bool pressed = (event.jbutton.state == SDL_PRESSED);

						if( ControlPort1 != NULL )
							ControlPort1->HandleEvent( pressed, buttonID );
						if( ControlPort2 != NULL )
							ControlPort2->HandleEvent( pressed, buttonID );
					}
					break ;

				/*
				case SDL_MOUSEMOTION:
					{
						unsigned char *pixels = (unsigned char*) screen->pixels;

						pixels += screen->pitch * event.motion.y;
						pixels += screen->format->BytesPerPixel * event.motion.x;

						int lum = (*(pixels++) + *(pixels++) + *(pixels++)) / 3;

						g->LightSense( event.motion.state != 0, lum > 64 );

						printf("%i\n",lum);
					}
					break ;
				*/
			}
		}
	
		// Main emulation loop
		if( machine != NULL )
		{
			unsigned short *frame = NULL;
			
			while( SDL_GetAudioStatus() == SDL_AUDIO_PLAYING &&
				  machine->AudioFull() ) 
				SDL_Delay(5);
			while( frame == NULL )
				frame = machine->Execute();

			filter->BlitFrame( screen, frame, PPU_PITCH );
		}
		else
		{
			SDL_Delay(100);
		}
	
		SDL_Flip( screen );
		ShowFPS();
	}
	while( running );

	SDL_Quit( );
	SaveConfiguration( argv[0], config );

	CloseDatabase();

	if( machine != NULL )
		delete machine;
	
	delete filter;
	
	if( ControlPort1 != NULL )
		delete ControlPort1;
	if( ControlPort2 != NULL )
		delete ControlPort2;

	return 0;
}