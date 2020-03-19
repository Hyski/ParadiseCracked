/*------------------------------------------------------*/
//     Class Name: CKeyboard
//     Coder: Naughty
//     Date: 23.03.2000 - ..
//     Company: Naughty
/*------------------------------------------------------*/
#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "DirectInputDevice.h"

typedef struct
{
	BYTE Front[256];
	BYTE Back[256];
	BYTE State[256];
} KEY_BUFFER;

class CKeyboard : public CDirectInputDevice
{
private:
	KEY_BUFFER m_Buffer;
protected:
	using CDirectInputDevice::m_lpDIDevice;
	using CDirectInputDevice::m_hDeviceEvent;
public:
	virtual void ProcessEvent(void);
public:
	CKeyboard();
	virtual ~CKeyboard();
public:
	void GetState(KEY_BUFFER *lpBuffer);
public:
	void Lock(void);
	void Unlock(void);
	//	доступно ли устройство
	bool IsLocked(void);
};

#endif