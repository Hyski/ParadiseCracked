#include "Precomp.h"
#include "DirectInputDevice.h"

CDirectInputDevice::CDirectInputDevice() : CNhtThread()
{
}

CDirectInputDevice::~CDirectInputDevice()
{
	Release();
}

void CDirectInputDevice::Release(void)
{
	if(m_lpDIDevice)
	{
		m_lpDIDevice->Unacquire();
		m_lpDIDevice->Release();
		m_lpDIDevice = NULL;
	}
	if(m_hDeviceEvent)
		CloseHandle(m_hDeviceEvent);
}

void CDirectInputDevice::Lock(void)
{
	if(m_lpDIDevice) m_lpDIDevice->Acquire();
}

void CDirectInputDevice::Unlock(void)
{
	if(m_lpDIDevice) m_lpDIDevice->Unacquire();
}

void CDirectInputDevice::ProcessEvent(void)
{
}

unsigned int CDirectInputDevice::Main(void)
{
//	console("- NhtDIDeviceThread created;\n");
	while(CanRun())
	{
		if(WaitForSingleObject(m_hDeviceEvent,1000)==WAIT_OBJECT_0)
		{
			ProcessEvent();
		}
	}
//	console("- NhtDIDeviceThread destroyed;\n");

	return 0;
}

bool CDirectInputDevice::IsLocked(void)
{
	return false;
}
