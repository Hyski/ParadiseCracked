/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
    Date:   03.07.2000

************************************************************************/
#include "Precomp.h"
#include "CharKeySupport.h"

//---------- Лог файл ------------
#ifdef _DEBUG
CLog key_log;
#define key	key_log["key.log"]
#else
#define key	/##/
#endif
//--------------------------------

#define KEYCHAR_NUMBER		63

namespace CharKeySupport
{
	//	буфер проверяемых скан кодов
	const unsigned char CharScanCodeBuffer[KEYCHAR_NUMBER] =
	{
		DIK_GRAVE,
		DIK_1,
		DIK_2,
		DIK_3,
		DIK_4,
		DIK_5,
		DIK_6,
		DIK_7,
		DIK_8,
		DIK_9,
		DIK_0,
		DIK_MINUS,
		DIK_EQUALS,
		DIK_BACKSLASH,
		DIK_Q,
		DIK_W,
		DIK_E,
		DIK_R,
		DIK_T,
		DIK_Y,   
		DIK_U,   
		DIK_I,   
		DIK_O,   
		DIK_P,   
		DIK_LBRACKET,
		DIK_RBRACKET,
		DIK_A,   
		DIK_S,   
		DIK_D,   
		DIK_F,   
		DIK_G,   
		DIK_H,   
		DIK_J,   
		DIK_K,   
		DIK_L,   
		DIK_SEMICOLON,   
		DIK_APOSTROPHE,   
		DIK_Z,
		DIK_X,   
		DIK_C,   
		DIK_V,   
		DIK_B,   
		DIK_N,   
		DIK_M,   
		DIK_COMMA,   
		DIK_PERIOD,	//	?
		DIK_SLASH,  
		DIK_NUMPAD1,   
		DIK_NUMPAD2,   
		DIK_NUMPAD3,   
		DIK_NUMPAD4,   
		DIK_NUMPAD5,   
		DIK_NUMPAD6,   
		DIK_NUMPAD7,   
		DIK_NUMPAD8,   
		DIK_NUMPAD9,   
		DIK_NUMPAD0,   
		DIK_SUBTRACT,
		DIK_ADD,
		DIK_DECIMAL,  
		DIK_DIVIDE, 
		DIK_MULTIPLY,
		DIK_SPACE
	};

	unsigned char KeyStateBuff[256];
}

void CharKeySupport::Init(void)
{
	GetKeyboardState(KeyStateBuff);
}

//	функция возвращает первую нажатую клавишу из вышеуказанного буфера
unsigned char CharKeySupport::ScanKeyboard(void)
{
	UINT VkCode;
	unsigned short ch;
	int ret;

	for(int i=0;i<KEYCHAR_NUMBER;i++)
	{
		if(Input::KeyFront(CharScanCodeBuffer[i]|Input::RESET))
		{
			key("ScanCode: %ld\n",CharScanCodeBuffer[i]);
			VkCode = MapVirtualKeyEx(CharScanCodeBuffer[i],1,GetKeyboardLayout(0));
			key("VkCode: %ld\n",VkCode);
			if(Input::KeyState(DIK_LSHIFT) || Input::KeyState(DIK_RSHIFT))
			{
				KeyStateBuff[16] = 129;
			}
			else
			{
				KeyStateBuff[16] = 0;
			}
			ret = ToAsciiEx(VkCode,CharScanCodeBuffer[i],(unsigned char *)KeyStateBuff,&ch,1,GetKeyboardLayout(0));
			key("ret: %ld\n",ret);
			if(ret!=0)
			{
				return ch;
			}
		}
	}

	return 0;
}
