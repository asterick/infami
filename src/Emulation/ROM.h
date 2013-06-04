#ifndef __NESPROGRAM_H
#define __NESPROGRAM_H

enum
{
	Horizontal,
	Vertical,
	FourScreen,
	ZeroPage,
	OnePage,
	MapperControlled
};

class ROM : public Famicom
{
public:
	ROM( const TimingSettings* Timing );
	virtual ~ROM();
	void LoadRam();
	void SetFileName( const char *fileName );
	virtual Famicom *GetSystem();
	virtual void Initalize( int Mirroring );

	bool BatteryBackup;

	int	 PrgMask;
	int	 RamMask;

	int  PRGSize;
	int  CHRSize;

	int  WRKSize;
	int  BCKSize;
	int  RAMSize;

	unsigned char *PRGRom;
	unsigned char *CHRRom;

	unsigned char *WRKRam;
	unsigned char *BCKRam;

	unsigned char *Trainer;

	unsigned char CHRRam[0x8000];

	const TimingSettings *Timing;
protected:
	char FileName[0x1000];			// 4k for filename
};

#endif