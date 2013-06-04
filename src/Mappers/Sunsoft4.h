#ifndef __SUNSOFT4_H
#define __SUNSOFT4_H

class Sunsoft4 : public ROM
{
public:
	Sunsoft4( const TimingSettings *Timing ) : ROM( Timing ) {}
	virtual void Initalize( int Mirroring );
protected:
	virtual unsigned char Read( unsigned short addr );
	virtual void Write( unsigned short addr, unsigned char ch );
	
	virtual void VideoAddressLatch( unsigned short Address );
	virtual void VideoWrite(  unsigned char Byte );
	virtual unsigned char VideoRead();
	
	Famicom *GetSystem() { return (Famicom*) this; }
private:
	void SetNameTables();

	unsigned char	m_BusTrash;
	unsigned short	m_VideoAddress;

	int				m_PRGMask;
	int				m_CHRMask;
	int				m_VROMMask;
	
	int				m_PRGBank;

	int				m_CHRBank0;
	int				m_CHRBank1;
	int				m_CHRBank2;
	int				m_CHRBank3;
	int				m_VROMBank0;
	int				m_VROMBank1;

	bool			m_VROMMode;

	int				Mirroring;
};

#endif