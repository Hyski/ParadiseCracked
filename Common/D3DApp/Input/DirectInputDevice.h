/*------------------------------------------------------*/
//     Class Name: CDirectInputDevice
//     Coder: Naughty
//     Date: 21.03.2000 - ..
//     Company: Naughty
/*------------------------------------------------------*/
#ifndef _DIRECT_INPUT_DEVICE_H_
#define _DIRECT_INPUT_DEVICE_H_

#include "NhtThread.h"

class CDirectInputDevice : public CNhtThread
{
public:
	enum {BUFFER_SIZE=16};
public:
	LPDIRECTINPUTDEVICE m_lpDIDevice;
	HANDLE	m_hDeviceEvent;
public:
	CDirectInputDevice();
	virtual ~CDirectInputDevice();
public:
	unsigned int Main(void);
	virtual void ProcessEvent(void);
public:
	void Release(void);
	virtual void Lock(void);
	virtual void Unlock(void);
	//	доступно ли устройство
	virtual bool IsLocked(void);
};

#endif