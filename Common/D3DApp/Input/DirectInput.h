/*------------------------------------------------------*/
//     Class Name: CDirectInput
//     Coder: Naughty
//     Date: 20.03.2000 - ..
//     Company: Naughty
/*------------------------------------------------------*/
#ifndef _DIRECT_INPUT_H_
#define _DIRECT_INPUT_H_

#include <dinput.h>

class CDirectInputDevice;

class CDirectInput
{
private:
	LPDIRECTINPUT7 m_lpDI;
public:
	CDirectInput();
	virtual ~CDirectInput();
public:
	BOOL Create(void);
	void Release(void);
public:
	BOOL InitMouse(HWND hMainWnd,CDirectInputDevice* lpDIDevice);
	BOOL InitKeyboard(HWND hMainWnd,CDirectInputDevice* lpDIDevice);

};

#endif