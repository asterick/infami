#include <memory.h>

#include "SDL2/SDL.h"

#include "../Emulation/Emulation.h"
#include "../System.h"
#include "DiskSystem.h"

DiskSystem::~DiskSystem()
{
	EjectDisk();
}

void DiskSystem::Initalize( int Mirroring )
{
	ROM::Initalize( Mirroring );

	m_DiskLoaded = false;
	m_WaveTableWritable = false;

	m_TimerCounter = 0xFFFF;
	m_TimerReload = 0xFFFF;
	m_TimerIRQEnable = false;
	m_TimerIRQActive = false;

	m_DiskInserted = false;

	m_DiskInserted = false;
	m_WriteProtected = false;
	m_CRCError = false;
	m_DataLost = false;
	m_DiskIRQActive = false;
	m_DiskIRQEnable = false;
	m_DataWrite = false;
	m_EnableCRC = false;
	m_GapResetCrc = false;

	// Head is somewhere near the middle of the disk
	m_CurrentSide		= 0;
	m_DriveWriteDelay	= 0;
}

void DiskSystem::SpecialKey( int Code )
{
	SDL_PauseAudio( true );

	switch( Code )
	{
	case SDLK_d:
		printf("Loading disk image. (Press CTRL+I to insert it)\n");
		m_DiskInserted = false;
		InsertDisk();
		break ;
	case SDLK_c:
		m_DiskInserted = false;
		m_CurrentSide	= (m_CurrentSide+1) % m_TotalSides;
		m_BaseAddress	= FDS_DISK_SIZE * m_CurrentSide;
		printf("Changed disk %i side to %c (Press CTRL+I to insert it)\n", (m_CurrentSide>>1)+1,(m_CurrentSide&1)+'A');
		break ;
	case SDLK_i:
		m_DiskInserted = m_DiskLoaded && !m_DiskInserted;
		printf("%s disk.\n", m_DiskInserted ? "Inserted" : "Ejected" );
		break ;
	}

	SDL_PauseAudio( false );
}

void DiskSystem::InsertDisk()
{
	const char *szName;

    szName = GetDiskFilename();
	if( !szName )
		return ;

	if( m_DiskLoaded )
		EjectDisk();

	// FDS Image Loader

	SDL_RWops *fo = SDL_RWFromFile(szName, "rb");

	if( fo == NULL )
		return ;

	SDL_RWseek(fo, 0, SEEK_END);
	int size = SDL_RWtell(fo);


	SDL_RWseek(fo, size % FDS_DISK_SIZE, SEEK_SET);	// Skip FDS headers
	size -= size % FDS_DISK_SIZE;

	m_BaseAddress = 0;
	m_CurrentSide = 0;
	m_TotalSides = size / FDS_DISK_SIZE;
	m_DiskImage = new unsigned char[size];
	m_Modified = new bool[size];

	for( int i = 0; i < m_TotalSides * FDS_DISK_SIZE; i++ )
		m_Modified[i] = false;

	SDL_RWread( fo, m_DiskImage, FDS_DISK_SIZE, m_TotalSides );

	SDL_RWclose(fo);

	m_DiskLoaded = true;

	// --- IPS AUTO LOADER ---

	GetFileName( m_SaveName, sizeof( m_SaveName), szName, ".ips" );
	fo = SDL_RWFromFile(m_SaveName, "rb");

	if( fo == NULL )
		return ;

	char header[5];

	SDL_RWread( fo, header, 5, 1 );

	if( header[0] != 'P' ||
		header[1] != 'A' ||
		header[2] != 'T' ||
		header[3] != 'C' ||
		header[4] != 'H' )
	{
		SDL_RWclose(fo);
		return ;
	}

	int offset = 0;
	size = 0;

	while( SDL_RWread( fo, &offset, 3, 1 ) > 0 )
	{
		SDL_RWread( fo, &size, 2, 1 );

		#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		int t = offset;
		offset =  (t & 0xFF000000) >> 24;
		offset |= (t & 0x00FF0000) >> 8;
		offset |= (t & 0x0000FF00) << 8;

		t = size;
		size =  (t & 0xFF000000) >> 24;
		size |= (t & 0x00FF0000) >> 8;
		#endif

		if( size == 0 )
		{
			unsigned char bytes = 0;
			unsigned char value = 0;

			SDL_RWread( fo, &bytes, 1, 1 );
			SDL_RWread( fo, &value, 1, 1 );

			// RLE Load with overflow checking
			while( bytes-- > 0 && offset < m_TotalSides * FDS_DISK_SIZE )
			{
				m_DiskImage[ offset ] = value;
				m_Modified[ offset ] = true;
				offset++;
			}
		}
		// RLE Load with overflow checking
		else if( size + offset <= m_TotalSides * FDS_DISK_SIZE )
		{
			SDL_RWread( fo, m_DiskImage + offset, size, 1 );
			for( int i = 0; i < size; i++ )
				m_Modified[ offset+i ] = true;
		}
	}

	SDL_RWclose(fo);
}

void DiskSystem::EjectDisk()
{
	if( m_DiskLoaded )
	{
		bool changed = false;

		for( int i = 0; i < m_TotalSides * FDS_DISK_SIZE; i++ )
		{
			if( m_Modified[i] )
			{
				changed = true ;
				break ;
			}
		}

		if( changed )
		{
			SDL_RWops *fo = SDL_RWFromFile(m_SaveName, "wb");

			if( fo == NULL )
				return ;

			SDL_RWwrite( fo, "PATCH", 5, 1 );

			int i = 0;
			while( i < m_TotalSides * FDS_DISK_SIZE )
			{
				if( m_Modified[i] )
				{
					int offset = i;
					while( (i-offset) < FDS_DISK_SIZE && m_Modified[++i] ) ;
					int size = i-offset;

					#if SDL_BYTEORDER == SDL_BIG_ENDIAN
					int t = offset;
					offset |= (t & 0x00FF0000) >> 8;
					offset |= (t & 0x0000FF00) << 8;
					offset |=  (t & 0x000000FF) << 24;

					t = size;
					size |= (t & 0x0000FF00) << 8;
					size =  (t & 0x000000FF) << 24;
					#endif

					SDL_RWwrite( fo, &offset, 3, 1 );
					SDL_RWwrite( fo, &size, 2, 1 );
					SDL_RWwrite( fo, m_DiskImage + offset, size, 1 );
				}
				else
				{
					i++ ;
				}
			}

			SDL_RWclose(fo);
		}

		delete m_DiskImage;
		delete m_Modified;
	}

	m_DiskLoaded = false;
}

// --- START FDS EMULATION HERE ---

// TODO: AUDIO CHANNELS
// TOOD: TIMER IRQ

bool DiskSystem::IRQActive()
{
	return m_TimerIRQActive || m_DiskIRQActive;
}

void DiskSystem::CPUClock()
{
	if( m_TimerIRQEnable && m_TimerCounter > 0 &&  --m_TimerCounter == 0)
	{
		// Underflow
		if( m_TimerIRQRepeat )
			m_TimerCounter = m_TimerReload;
		m_TimerIRQActive = true;
	}

	// --- FDS DRIVE ---

	// TODO: REPLACE THE FDS CODE HERE
	if( (m_DriveIRQCountDown > 0) && (--m_DriveIRQCountDown == 0) && m_DiskIRQEnable )
		m_DiskIRQActive = true;

	// TODO: FDS Audio
}

unsigned char DiskSystem::FDSRead( unsigned short addr, unsigned char ch )
{
	if( addr >= 0x4040 )
	{
		if( !m_SoundIOEnable )
			return ch;

		if( addr <= 0x407F )
			return m_WaveTableRam[addr&0x3F] = ch;
	}

	if( addr >= 0x4024 && !m_DiskIOEnable )
		return ch;

	switch( addr )
	{
		// --- DISK I/O REGISTERS --------------------------

		case 0x4030:		// Disk Status Register 0 (R)
			{
				unsigned char status =
					(m_TimerIRQActive	? 0x01 : 0) |
					(m_DiskIRQActive	? 0x02 : 0) |
					(m_CRCError			? 0x10 : 0) |
					(m_DataLost			? 0x40 : 0);
				m_DiskIRQActive = false;
				m_TimerIRQActive = false;
				m_CRCError = false;
				m_DataLost = false;
				return status ;
			}

		case 0x4031:		// Disk Data Read Register (R)
			m_DiskIRQActive = false;
			m_DataAvailable = false;

			m_DriveIRQCountDown = FDS_CYCLES_PER_BYTE;

			if( !(m_DrivePointer & 0xFF) )
				printf("FDS Read: %i:%i  %i\n", m_CurrentSide, m_DrivePointer, m_DriveIRQCountDown);
			ch = m_DiskImage[m_BaseAddress+m_DrivePointer];

			if( m_DrivePointer < FDS_DISK_SIZE-1 )
				m_DrivePointer++;

			return ch;

		case 0x4032:		// Disk Status Register 1 (R)
			return
				(m_DiskInserted		? 0 : 0x01 ) |
				(m_DriveReady		? 0 : 0x02 ) |
				((m_WriteProtected || m_DriveStopRequest || m_DriveStartRequest) ? 0x04 : 0 );

		case 0x4033:		// Disk External Connector Input (R)
			return m_DriveStarted ? 0x80 : 0;	// Return the status of the motor (battery check)

		// --- AUDIO REGISTERS -----------------------------

		case 0x4090:		// Sound Current Volume Gain Level (6bit) (R)
			break ;
		case 0x4092:		// Sound Current Sweep Gain Level (6bit) (R)
			break ;
	}

	return ch;
}

void DiskSystem::FDSWrite( unsigned short addr, unsigned char ch )
{
	if( addr >= 0x4040 )
	{
		if( !m_SoundIOEnable )
			return ;

		if( addr <= 0x407F )
		{
			if( m_WaveTableWritable )
				m_WaveTableRam[addr&0x3F] = ch & 0x3F;

			return ;
		}
	}

	if( addr >= 0x4024 && !m_DiskIOEnable )
		return ;

	switch( addr )
	{
		case 0x4020:		// Timer IRQ Counter Reload value LSB (W)
			m_TimerReload = (m_TimerReload & 0xFF00) | ch;
			m_TimerIRQActive = false;
			break ;
		case 0x4021:		// Timer IRQ Counter Reload value MSB (W)
			m_TimerReload = (m_TimerReload & 0x00FF) | (ch << 8);
			m_TimerIRQActive = false;
			break ;
		case 0x4022:		// Timer IRQ Enable/Disable (W)
			m_TimerCounter = m_TimerReload;
			m_TimerIRQEnable = (ch & 2) != 0;
			m_TimerIRQRepeat = (ch & 1) != 0;
			m_TimerIRQActive = false;
			break ;

		case 0x4023:		// 2C33 I/O Control Port
			m_DiskIOEnable = (ch & 1) != 0;
			m_SoundIOEnable = (ch & 2) != 0;
			break ;

		// --- DISK I/O REGISTERS --------------------------

		case 0x4024:		// Disk Data Write Register (W)
			//m_DiskIRQActive = false;
			m_DataLost = false;
			m_DataAvailable = false;

			if( m_DiskInserted && m_DiskIOEnable && m_DataWrite )
			{
				if( m_DrivePointer >= 2 && m_DrivePointer <= FDS_DISK_SIZE+2 )
				{
					//printf("FDS Write: %i:%i\n", m_CurrentSide, m_DrivePointer-2);
					if( m_DriveWriteDelay > 0 )
						m_DriveWriteDelay--;
					else if( m_DiskImage[m_BaseAddress+m_DrivePointer-2] != ch )
					{

						m_DiskImage[m_BaseAddress+m_DrivePointer-2] = ch;
						m_Modified[m_BaseAddress+m_DrivePointer-2] = true;
					}
				}
			}

			break ;
		case 0x4025:		// Disk Control Register (W)
			{
				m_DiskIRQActive = false;

				if( ch & 0x8 )
					SetMirrorHorizontal();
				else
					SetMirrorVertical();

				if( !m_DiskIOEnable )
					break ;

				bool m_PreviousGap = m_GapResetCrc;

				m_DriveStopRequest	= (ch & 0x01) == 0;
				m_DriveStartRequest	= (ch & 0x02) == 0;

				m_DataWrite		= (ch & 0x04) == 0;
				m_EnableCRC		= (ch & 0x10) != 0;
				// UNKNOWN BIT 0x20
				m_GapResetCrc	= (ch & 0x40) != 0;
				m_DiskIRQEnable	= (ch & 0x80) != 0;

				// Bunch of nintendulator hacks

				if( !m_GapResetCrc )
				{
					if( m_PreviousGap && !m_EnableCRC )
					{
						//printf("FDS CRC Rewind\n");
						m_DrivePointer -= 2;
						m_DriveIRQCountDown = FDS_GAP_BITS(947);
					}

					if( m_DrivePointer < 0 )
						m_DrivePointer = 0;
				}

				if( m_DataWrite )
					m_DriveWriteDelay = 2;

				if( !m_DriveStartRequest )
				{
					//printf("FDS Rewind\n", m_CurrentSide, m_DrivePointer-2);
					m_DrivePointer = 0;
					m_DriveIRQCountDown = FDS_GAP_BITS(42000);
				}

				if( m_GapResetCrc )
					m_DriveIRQCountDown = FDS_GAP_BITS(947);
			}

			break ;
		case 0x4026:		// Disk External Connector Output (W)
			// EXT Port unused for now
			break ;

		// --- AUDIO REGISTERS -----------------------------

		case 0x4080:		// Sound Volume Envelope (W)
		case 0x4082:		// Sound Wave RAM Sample Rate LSB (W)
		case 0x4083:		// Sound Wave RAM Sample Rate MSB and Control (W)
		case 0x4084:		// Sound Sweep Envelope (W)
		case 0x4085:		// Sound Sweep Bias (W)
		case 0x4086:		// Sound Modulation Frequency LSB (W)
		case 0x4087:		// Sound Modulation Frequency MSB (W)
		case 0x4088:		// Sound Modulation Table (W)
		case 0x4089:		// Sound Wave RAM Control (W)
		case 0x408A:		// Sound Envelope Base Frequency (W)
			break ;
	}
}

// --- BUS STUFF --

unsigned char DiskSystem::Read( unsigned short addr )
{
	switch( addr & 0xE000 )
	{
	case 0x0000:
		return m_BusTrash = SystemRam[addr&0x7FF];
	case 0x2000:
		return m_BusTrash = PPURead( addr, m_BusTrash );
	case 0x4000:
		if( addr >= 0x4020 )
			return m_BusTrash = FDSRead( addr, m_BusTrash );
		return m_BusTrash = APURead( addr, m_BusTrash );

	case 0x6000:
	case 0x8000:
	case 0xA000:
	case 0xC000:
		return m_Memory[addr & 0x7FFF];
	case 0xE000:
		return m_BusTrash = PRGRom[addr & PrgMask];
	}

	return m_BusTrash;
}

void DiskSystem::Write( unsigned short addr, unsigned char ch )
{
	m_BusTrash = ch;
	switch( addr & 0xE000 )
	{
	case 0x0000:
		SystemRam[addr&0x7FF] = ch;
		break ;
	case 0x2000:
		PPUWrite( addr, m_BusTrash );
		break ;
	case 0x4000:
		if( addr >= 0x4020 )
			FDSWrite( addr, m_BusTrash );
		APUWrite( addr, m_BusTrash );
		break ;
	case 0x6000:
	case 0x8000:
	case 0xA000:
	case 0xC000:
		m_Memory[addr & 0x7FFF] = m_BusTrash;
		break ;
	}
}

void DiskSystem::VideoAddressLatch( unsigned short Address )
{
	m_VideoAddress = Address;
}

void DiskSystem::VideoWrite( unsigned char Byte )
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	//
		CHRRam[m_VideoAddress&0x1FFF] = Byte;
		break ;
	default:		// Name tables
		SetNameTable( m_VideoAddress, Byte );
		break ;
	}
}

unsigned char DiskSystem::VideoRead()
{
	switch( m_VideoAddress & 0x3000 )
	{
	case 0x0000:	// CHR-ROM
	case 0x1000:	//
		return CHRRam[m_VideoAddress&0x1FFF];
	default:		// Name tables
		return GetNameTable( m_VideoAddress );
	}
	return 0;
}
