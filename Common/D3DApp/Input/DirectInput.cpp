#include "Precomp.h"
#include "DirectInputDevice.h"
#include "DirectInput.h"

CDirectInput::CDirectInput()
{
	m_lpDI = NULL;
}

CDirectInput::~CDirectInput()
{
	Release();
}

BOOL CDirectInput::Create(void)
{
	if(!FAILED(DirectInputCreateEx(GetModuleHandle(NULL),
								  DIRECTINPUT_VERSION,
								  IID_IDirectInput7,
								  (void **)&m_lpDI,
								  NULL)))
	{
//		console("- DirectInput created;\n");
		return TRUE;
	}

	return FALSE;
}

BOOL CDirectInput::InitMouse(HWND hMainWnd,CDirectInputDevice* lpDIDevice)
{
	//	создаем устройство: мышь
	if(FAILED(m_lpDI->CreateDevice(GUID_SysMouse,&lpDIDevice->m_lpDIDevice,NULL)))
		return FALSE;     
//	console("- CreateDevice(Mouse) - ok!\n");
	//	устанавливаем форат передаваемых данных: мышинный
	if(FAILED(lpDIDevice->m_lpDIDevice->SetDataFormat(&c_dfDIMouse)))
		return FALSE;     
//	console("- SetDataFormat(Mouse) - ok!\n");
		//	устанавливаем поведение устройства
	if(FAILED(lpDIDevice->m_lpDIDevice->SetCooperativeLevel(hMainWnd,DISCL_EXCLUSIVE|DISCL_FOREGROUND)))
		return FALSE;     
//	console("- SetCooperativeLevel(Mouse: DISCL_EXCLUSIVE|DISCL_FOREGROUND) - ok!\n");
	lpDIDevice->m_hDeviceEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	if(NULL == lpDIDevice->m_hDeviceEvent)
		return FALSE;
//	console("- CreateEvent(m_hMouseEvent) - ok!\n");
	if(FAILED(lpDIDevice->m_lpDIDevice->SetEventNotification(lpDIDevice->m_hDeviceEvent)))
		return FALSE;
//	console("- SetEventNotification(m_hMouseEvent) - ok!\n");
	// setup the buffer size for the mouse data
	DIPROPDWORD dipdw;
	
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = CDirectInputDevice::BUFFER_SIZE;
	
	if(FAILED(lpDIDevice->m_lpDIDevice->SetProperty(DIPROP_BUFFERSIZE,&dipdw.diph)))
		return FALSE;
//	console("- SetProperty(Mouse) - ok!\n");

	return TRUE;
}

BOOL CDirectInput::InitKeyboard(HWND hMainWnd,CDirectInputDevice* lpDIDevice)
{
	//	создаем устройство: клавиатура
	if(FAILED(m_lpDI->CreateDevice(GUID_SysKeyboard,&lpDIDevice->m_lpDIDevice,NULL)))
		return FALSE;     
//	console("- CreateDevice( Keyboard ) - ok!\n");
	//	устанавливаем формат передаваемых данных: клавиатурный
	if(FAILED(lpDIDevice->m_lpDIDevice->SetDataFormat(&c_dfDIKeyboard)))
		return FALSE;     
//	console("- SetDataFormat( Keyboard ) - ok!\n");
	//	устанавливаем поведение устройства
	if(FAILED(lpDIDevice->m_lpDIDevice->SetCooperativeLevel(hMainWnd,DISCL_FOREGROUND|DISCL_EXCLUSIVE|DISCL_NOWINKEY)))
		return FALSE;     
//	console("- SetCooperativeLevel( Keyboard ) - ok!\n");
	//	создаем событие клавиатурное
	lpDIDevice->m_hDeviceEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	if(NULL == lpDIDevice->m_hDeviceEvent)
		return FALSE;
//	console("- m_hKeyboardEvent create - ok;\n");
	//	связываем событие с устройством
	if(FAILED(lpDIDevice->m_lpDIDevice->SetEventNotification(lpDIDevice->m_hDeviceEvent)))
		return FALSE;     
	//	устанавливаем свойства для клавиатуры
    DIPROPDWORD dipdw;

    dipdw.diph.dwSize = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj = 0;
    dipdw.diph.dwHow = DIPH_DEVICE;
    dipdw.dwData = CDirectInputDevice::BUFFER_SIZE;

    if(FAILED(lpDIDevice->m_lpDIDevice->SetProperty(DIPROP_BUFFERSIZE,&dipdw.diph)))
		return FALSE;

	return TRUE;
}

void CDirectInput::Release(void)
{
	RELEASE(m_lpDI);
//	console("- DirectInput - released;\n");
}
