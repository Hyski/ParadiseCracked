/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
    Date:   10.05.2000

************************************************************************/
#ifndef _INPUT_H_
#define _INPUT_H_

#include "DirectInput.h"
#include "Mouse.h"
#include "Keyboard.h"

namespace Input
{
	enum {RESET = 0x800000};						//	сбрасываать проверяемое значение в 0
	//-----------------------------------------------
	BOOL Init(HWND hMainWnd);
	void Close(void);
	void Update(void);
	//-----------------------------------------------
	inline CMouse* Mouse(void);
	inline const MOUSE_STATE& MouseState(void);
	inline CKeyboard* Keyboard(void);
	inline int KeyFront(unsigned int index);
	inline int KeyBack(unsigned int index);
	inline int KeyState(unsigned int index);
	inline const KEY_BUFFER* KeyboardState(void);
	inline unsigned char* KeyStateBuffer(void);
	unsigned int GetDoubleClickTime(void);
	//	информация о состоянии
	inline BOOL IsInit(void);
}

//--------------------------------------------------------------
inline CMouse* Input::Mouse(void)
{
	extern CMouse* m_pMouse;
	return m_pMouse;
}

inline CKeyboard* Input::Keyboard(void)
{
	extern CKeyboard* m_pKeyboard;
	return m_pKeyboard;
}

inline const MOUSE_STATE& Input::MouseState(void)
{
	extern MOUSE_STATE m_MouseState;
	return m_MouseState;
}

inline int Input::KeyFront(unsigned int index)
{
	extern KEY_BUFFER m_KeyboardState;
	BYTE value = m_KeyboardState.Front[index&0xff];
	if(index&RESET && value)
		m_KeyboardState.Front[index&0xff] = 0;
	return value;
}

inline int Input::KeyBack(unsigned int index)
{
	extern KEY_BUFFER m_KeyboardState;
	BYTE value = m_KeyboardState.Back[index&0xff];
	if(index&RESET && value)
		m_KeyboardState.Back[index&0xff] = 0;
	return value;
}

inline int Input::KeyState(unsigned int index)
{
	extern KEY_BUFFER m_KeyboardState;
	BYTE value = m_KeyboardState.State[index&0xff];
	if(index&RESET && value)
		m_KeyboardState.State[index&0xff] = 0;
	return value;
}

inline unsigned char* Input::KeyStateBuffer(void)
{
	extern KEY_BUFFER m_KeyboardState;
	return m_KeyboardState.State;
}

inline BOOL Input::IsInit(void)
{
	extern BOOL m_bInit;
	return m_bInit;
}

inline const KEY_BUFFER* Input::KeyboardState(void)
{
	extern KEY_BUFFER m_KeyboardState;
	return &m_KeyboardState;
}

#endif