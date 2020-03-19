#include "Precomp.h"
#include "Keyboard.h"

CRITICAL_SECTION KeyboardDataCS;

CKeyboard::CKeyboard() : CDirectInputDevice()
{
	memset(&m_Buffer,0,sizeof(KEY_BUFFER));
	InitializeCriticalSection(&KeyboardDataCS);
}

CKeyboard::~CKeyboard()
{
	Stop();
	DeleteCriticalSection(&KeyboardDataCS);
}

void CKeyboard::ProcessEvent(void)
{
	HRESULT hResult;
    DIDEVICEOBJECTDATA od[BUFFER_SIZE];
    DWORD dwElements = BUFFER_SIZE;

    while(m_lpDIDevice) 
    {
        hResult = m_lpDIDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), 
													 od,
													 &dwElements, 
													 0);
		//	error or no data available
		if(FAILED(hResult) || (dwElements==0)) 
			break;
		EnterCriticalSection(&KeyboardDataCS);
        for(DWORD i=0;i<dwElements;i++) 
        {
			if(m_Buffer.State[od[i].dwOfs] = (BYTE)(od[i].dwData & 0x80))
				m_Buffer.Front[od[i].dwOfs]++;
			else
				m_Buffer.Back[od[i].dwOfs]++;
		}	
		LeaveCriticalSection(&KeyboardDataCS);
	}
}

void CKeyboard::GetState(KEY_BUFFER *lpBuffer)
{
	EnterCriticalSection(&KeyboardDataCS);
	memcpy(lpBuffer,&m_Buffer,sizeof(KEY_BUFFER));
	memset(&m_Buffer.Front,0,sizeof(m_Buffer.Front));
	memset(&m_Buffer.Back,0,sizeof(m_Buffer.Back));
	LeaveCriticalSection(&KeyboardDataCS);
}

void CKeyboard::Lock(void)
{
	//	reset data
	memset(&m_Buffer.State,0,sizeof(m_Buffer.State));
	//	----------
	CDirectInputDevice::Lock();
}

void CKeyboard::Unlock(void)
{
	//	reset data
	memset(&m_Buffer.State,0,sizeof(m_Buffer.State));
	//	----------
	CDirectInputDevice::Unlock();
}
//	доступно ли устройство
bool CKeyboard::IsLocked(void)
{
	unsigned char buffer[256];

	if(m_lpDIDevice) return !FAILED(m_lpDIDevice->GetDeviceState(256,buffer));
	else return false;
}
