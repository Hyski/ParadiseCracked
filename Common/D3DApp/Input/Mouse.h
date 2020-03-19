/*------------------------------------------------------*/
//     Class Name: CMouse
//     Coder: Naughty
//     Date: 20.03.2000 - ..
//     Company: Naughty
/*------------------------------------------------------*/
#ifndef _MOUSE_H_
#define _MOUSE_H_

#include "DirectInputDevice.h"

typedef struct
{
	float x,y;
	float dx,dy,dz;
	int LButtonFront;
	int LButtonBack;
	int LButtonState;
	int RButtonFront;
	int RButtonBack;
	int RButtonState;
	int MButtonFront;
	int MButtonBack;
	int MButtonState;
	int LDblClick;
	int RDblClick;
	int MDblClick;
} MOUSE_STATE;

class CMouse : public CDirectInputDevice
{
private:
	struct ClickInfo
	{
		unsigned int uiClickCount;	//	кол-во тиков 
		float x,y;					//	место где произошел клик
	};
private:
	float m_Sensitivity;
	MOUSE_STATE m_MS;
	RECT m_rcBorder;
	bool m_bLockPosition;	//	привязать мышь к текущей позиции
public:	//	касаемо DoubleClick'а
	unsigned int m_uiDoubleClickTime;		//	системные установки
private:
	bool m_bLBtnClick;						//	флаг того что, кнопка была нажата
	bool m_bRBtnClick;						//	флаг того что, кнопка была нажата
	bool m_bMBtnClick;						//	флаг того что, кнопка была нажата
	ClickInfo m_ciLButton;					//	ticks from last mouse button pressed
	ClickInfo m_ciRButton;					//	ticks from last mouse button pressed
	ClickInfo m_ciMButton;					//	ticks from last mouse button pressed
protected:
	using CDirectInputDevice::m_lpDIDevice;
	using CDirectInputDevice::m_hDeviceEvent;
public:
	CMouse();
	virtual ~CMouse();
public:
	virtual void ProcessEvent(void);
public:
	void GetState(MOUSE_STATE *lpMS);
	void SetSensitivity(float Sensitivity);
	void SetBorder(RECT *lpRect);
	void SetBorder(int left,int top,int right,int bottom);
	void SetPos(int x,int y);
public:
	void Lock(void);
	void Unlock(void);
	//	доступно ли устройство
	bool IsLocked(void);
public:
	void LockPosition(void);
	void UnlockPosition(void);
public:
	//	узнать время отлавливания DoubleClick'а
	unsigned int DoubleClickTime(void);
private:
	void UpdateMousePos(int dx,int dy,int dz);
};

inline void CMouse::LockPosition(void)
{
	m_bLockPosition = true;
}

inline void CMouse::UnlockPosition(void)
{
	m_bLockPosition = false;
}

inline unsigned int CMouse::DoubleClickTime(void)
{
	return m_uiDoubleClickTime;
}

#endif