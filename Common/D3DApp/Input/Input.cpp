/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
    Date:   10.05.2000

************************************************************************/
#include "Precomp.h"
#include "Input.h"
#include "CharKeySupport.h"

//---------- Лог файл ------------
#ifdef _DEBUG_SHELL
CLog dinput_log;
#define dinput	dinput_log["dinput.log"]
#else
#define dinput	/##/
#endif
//--------------------------------

namespace Input
{
	CDirectInput *m_pDirectInput = NULL;
	CMouse *m_pMouse = NULL;
	CKeyboard *m_pKeyboard = NULL;
	//-----------------------------
	MOUSE_STATE m_MouseState;
	KEY_BUFFER m_KeyboardState;
	//-----------------------------
	BOOL m_bInit = FALSE;
}


BOOL Input::Init(HWND hMainWnd)
{
	memset(&m_MouseState,0,sizeof(MOUSE_STATE));
	memset(&m_KeyboardState,0,sizeof(KEY_BUFFER));

	//	поддержка символьного ввода
	CharKeySupport::Init();
	//	---------------------------
	m_pDirectInput = new CDirectInput();
	m_pMouse = new CMouse();
	m_pKeyboard = new CKeyboard();
	if(m_pDirectInput && m_pMouse && m_pKeyboard)
	{
		if(m_pDirectInput->Create())
		{
			if(m_pDirectInput->InitMouse(hMainWnd,m_pMouse) &&
			   m_pDirectInput->InitKeyboard(hMainWnd,m_pKeyboard))
			{
				return m_bInit = TRUE;
			}
		}
	}

	return FALSE;
}

void Input::Close(void)
{
	m_bInit = FALSE;
	dinput("    ...enter to CInput::Release();\n");
	DESTROY(m_pKeyboard);
	dinput("    - destroy m_lpKeyboard;\n");
	DESTROY(m_pMouse);
	dinput("    - destroy m_lpMouse;\n");
	DESTROY(m_pDirectInput);
	dinput("    - destroy m_lpDirectInput;\n");
	dinput("    ...leave from CInput::Release();\n");
}

void Input::Update(void)
{
	m_pMouse->GetState(&m_MouseState);
	m_pKeyboard->GetState(&m_KeyboardState);
}

unsigned int Input::GetDoubleClickTime(void)
{
	return m_pMouse->m_uiDoubleClickTime;
}
