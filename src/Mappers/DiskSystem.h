#ifndef __DISKSYSTEM_H
#define __DISKSYSTEM_H

// 148 CPU cycles to deserialize one byte
#define FDS_CYCLES_PER_BYTE		(133)	// FDS max speed (148)
#define FDS_DISK_SIZE			(65500)
#define FDS_GAP_BITS(x)			(FDS_CYCLES_PER_BYTE*x/8)

class DiskSystem : public ROM
{
public:
	DiskSystem( const TimingSettings *Timing ) : ROM( Timing ) {}
	~DiskSystem();
	virtual void Initalize( int Mirroring );

	void SpecialKey( int Code );
private:
	void InsertDisk();
	unsigned char FDSRead( unsigned short addr, unsigned char ch );
	void FDSWrite( unsigned short addr, unsigned char ch );
	void CPUClock();
	void EjectDisk();
protected:
	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	virtual void VideoWrite(  unsigned char Byte );
	virtual unsigned char VideoRead();
	virtual bool IRQActive();
	Famicom *GetSystem() { return (Famicom*) this; }

	unsigned char	m_BusTrash;
	unsigned char	m_Memory[0x8000];

	unsigned short	m_VideoAddress;

	bool			m_DiskLoaded;

	char			m_SaveName[512];
	unsigned char	*m_DiskImage;
	bool			*m_Modified;
	int				m_CurrentSide;
	int				m_TotalSides;
	int				m_BaseAddress;

	unsigned char	m_WaveTableRam[0x40];
	bool			m_WaveTableWritable;

	int				m_TimerCounter;
	int				m_TimerReload;
	bool			m_TimerIRQEnable;
	bool			m_TimerIRQActive;
	bool			m_TimerIRQRepeat;

	bool			m_DiskIOEnable;
	bool			m_SoundIOEnable;

	// Disk access variables

	bool			m_DriveStarted;			// 
	bool			m_DriveReady;			//
	bool			m_DriveStopRequest;		//
	bool			m_DriveStartRequest;	//

	bool			m_DiskInserted;			// Disk inserted
	bool			m_WriteProtected;		// This is almost always false

	bool			m_CRCError;				// Disk
	bool			m_DataAvailable;
	bool			m_DataLost;
	
	bool			m_DiskIRQActive;
	bool			m_DiskIRQEnable;
	bool			m_DataWrite;
	bool			m_EnableCRC;
	bool			m_GapResetCrc;
	bool			m_RewindCycle;

	int				m_DriveIRQCountDown;
	int				m_DrivePointer;
	int				m_DriveWriteDelay;
};

#endif