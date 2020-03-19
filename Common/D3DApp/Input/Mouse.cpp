#include "Precomp.h"
#include "Mouse.h"

CRITICAL_SECTION MouseDataCS;

CMouse::CMouse() : CDirectInputDevice()
{
	m_Sensitivity = 1.7f;
	memset(&m_MS,0,sizeof(MOUSE_STATE));
	m_rcBorder.left = -1;
	m_rcBorder.top = -1;
	m_rcBorder.right = 641;
	m_rcBorder.bottom = 481;
	m_bLockPosition = false;
	//	-------------------------------------------------
	m_uiDoubleClickTime = GetDoubleClickTime();
	m_bLBtnClick = false;
	m_bRBtnClick = false;
	m_bMBtnClick = false;

	memset(&m_ciLButton,0,sizeof(ClickInfo));
	memset(&m_ciRButton,0,sizeof(ClickInfo));
	memset(&m_ciMButton,0,sizeof(ClickInfo));

	//	-------------------------------------------------
	InitializeCriticalSection(&MouseDataCS);
}

CMouse::~CMouse()
{
	Stop();
	DeleteCriticalSection(&MouseDataCS);
}

void CMouse::ProcessEvent(void)
{
	HRESULT hResult;
    DIDEVICEOBJECTDATA  od;
    DWORD dwElements = 1;

    while(m_lpDIDevice) 
    {
        hResult = m_lpDIDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), 
													 &od,
													 &dwElements, 
													 0);
		//	error or no data available
		if(FAILED(hResult) || (dwElements==0)) 
			break;
        // look at the element to see what happened
		EnterCriticalSection(&MouseDataCS);
		switch(od.dwOfs)
		{
            case DIMOFS_X:       // Mouse horizontal motion 
				UpdateMousePos(od.dwData,0,0);
                break;
            case DIMOFS_Y:       // Mouse vertical motion 
				UpdateMousePos(0,od.dwData,0);
                break;
            case DIMOFS_Z:       // Mouse z-axis motion 
				UpdateMousePos(0,0,od.dwData);
                break;
			case DIMOFS_BUTTON0: 
				if((m_MS.LButtonState = (od.dwData & 0x80)))	//	button down
				{
					m_MS.LButtonFront++;
					//	проверка на DblClick
					if(((od.dwTimeStamp-m_ciLButton.uiClickCount)<m_uiDoubleClickTime) &&
						((m_ciLButton.x == m_MS.x) && (m_ciLButton.y == m_MS.y)) &&
						m_bLBtnClick)
					{
						m_MS.LDblClick = 1;
						m_bLBtnClick = false;
					}
					else
					{
						m_bLBtnClick = true;
					}
					m_ciLButton.uiClickCount = od.dwTimeStamp;
					m_ciLButton.x = m_MS.x;
					m_ciLButton.y = m_MS.y;
				}
				else
					m_MS.LButtonBack++;
				break;
            case DIMOFS_BUTTON1: 
				if((m_MS.RButtonState = (od.dwData & 0x80)))	//	button down
				{
					m_MS.RButtonFront++;
					//	проверка на DblClick
					if(((od.dwTimeStamp-m_ciRButton.uiClickCount)<m_uiDoubleClickTime) &&
						((m_ciRButton.x == m_MS.x) && (m_ciRButton.y == m_MS.y)) &&
						m_bRBtnClick)
					{
						m_MS.RDblClick = 1;
						m_bRBtnClick = false;
					}
					else
					{
						m_bRBtnClick = true;
					}
					m_ciRButton.uiClickCount = od.dwTimeStamp;
					m_ciRButton.x = m_MS.x;
					m_ciRButton.y = m_MS.y;
				}
				else
					m_MS.RButtonBack++;
				break;
            case DIMOFS_BUTTON2: 
				if((m_MS.MButtonState = (od.dwData & 0x80)))	//	button down
				{
					//	проверка на DblClick
					if(((od.dwTimeStamp-m_ciMButton.uiClickCount)<m_uiDoubleClickTime) &&
						((m_ciMButton.x == m_MS.x) && (m_ciMButton.y == m_MS.y)) &&
						m_bMBtnClick)
					{
						m_MS.MDblClick = 1;
						m_bMBtnClick = false;
					}
					else
					{
						m_bMBtnClick = true;
					}
					m_ciMButton.uiClickCount = od.dwTimeStamp;
					m_ciMButton.x = m_MS.x;
					m_ciMButton.y = m_MS.y;
				}
				else
					m_MS.MButtonBack++;
				break;
		}
		LeaveCriticalSection(&MouseDataCS);
	}
}

void CMouse::UpdateMousePos(int dx,int dy,int dz)
{
	m_MS.dx += (float)dx*m_Sensitivity;
	m_MS.dy += (float)dy*m_Sensitivity;
	m_MS.dz += (float)dz;
	if(!m_bLockPosition)
	{
		m_MS.x += (float)dx*m_Sensitivity;
		m_MS.y += (float)dy*m_Sensitivity;
        m_MS.x = std::min<float>(std::max<float>(m_MS.x,m_rcBorder.left),m_rcBorder.right);
		m_MS.y = std::min<float>(std::max<float>(m_MS.y,m_rcBorder.top),m_rcBorder.bottom);
	}
}

void CMouse::GetState(MOUSE_STATE *lpMS)
{
	EnterCriticalSection(&MouseDataCS);

	memcpy(lpMS,&m_MS,sizeof(MOUSE_STATE));
	//	reset data
	m_MS.LButtonFront = 0;
	m_MS.LButtonBack  = 0;
	m_MS.RButtonFront = 0;
	m_MS.RButtonBack  = 0;
	m_MS.MButtonFront = 0;
	m_MS.MButtonBack  = 0;
	m_MS.LDblClick = 0;
	m_MS.RDblClick = 0;
	m_MS.MDblClick = 0;

	m_MS.dx = 0;
	m_MS.dy = 0;
	m_MS.dz = 0;

	LeaveCriticalSection(&MouseDataCS);
}

void CMouse::SetSensitivity(float Sensitivity)
{
	EnterCriticalSection(&MouseDataCS);

	m_Sensitivity = Sensitivity;

	LeaveCriticalSection(&MouseDataCS);
}

void CMouse::SetPos(int x,int y)
{
	EnterCriticalSection(&MouseDataCS);

	m_MS.x = (float)x;
	m_MS.y = (float)y;

	LeaveCriticalSection(&MouseDataCS);
}

void CMouse::SetBorder(RECT *lpRect)
{
	EnterCriticalSection(&MouseDataCS);

	memcpy(&m_rcBorder,lpRect,sizeof(RECT));

	LeaveCriticalSection(&MouseDataCS);
}

void CMouse::SetBorder(int left,int top,int right,int bottom)
{
	EnterCriticalSection(&MouseDataCS);

	m_rcBorder.left = left;
	m_rcBorder.top = top;
	m_rcBorder.right = right;
	m_rcBorder.bottom = bottom;

	LeaveCriticalSection(&MouseDataCS);
}

void CMouse::Lock(void)
{
	//	reset data
	m_MS.LButtonState = 0;
	m_MS.RButtonState = 0;
	m_MS.MButtonState = 0;
	m_MS.LDblClick = 0;
	m_MS.RDblClick = 0;
	m_MS.MDblClick = 0;
	//	----------
	CDirectInputDevice::Lock();
}

void CMouse::Unlock(void)
{
	//	reset data
	m_MS.LButtonState = 0;
	m_MS.RButtonState = 0;
	m_MS.MButtonState = 0;
	m_MS.LDblClick = 0;
	m_MS.RDblClick = 0;
	m_MS.MDblClick = 0;
	//	----------
	CDirectInputDevice::Unlock();
}
//	доступно ли устройство
bool CMouse::IsLocked(void)
{
	DIMOUSESTATE dims;

	if(m_lpDIDevice) return !FAILED(m_lpDIDevice->GetDeviceState(sizeof(dims),&dims));
	else return false;
}
