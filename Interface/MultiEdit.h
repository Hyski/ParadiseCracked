/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   30.10.2001

************************************************************************/
#ifndef _MULTI_EDIT_H_
#define _MULTI_EDIT_H_

#include "../Common/UI/Widget.h"

class TextBox;
class Edit;

class MultiEdit : public Widget
{
	class EditController;
	class Service;
	friend EditController;
public:
	struct Traits
	{
		const char* m_CaretImageName;
	};
private:
	TextBox* m_TextBox;						//	текст бокс
	Edit* m_Edit;							//	строка редактирования
	unsigned int m_EditOffset;				//	смещение строки редактирования
	EditController* m_EController;
public:
	MultiEdit(const char* name);
	virtual ~MultiEdit();
public:
	//	установить размеры
	void SetRegion(int left,int top,int right,int bottom);
	//	установить шрифт
	void SetFont(const char* name,int color);
	//	очистить текст
	void Clear(void);
	//	установить свойства
	void SetTraits(const Traits& t);
	//	получить текст
	const std::string& GetText(void) const;
private:
	void OnReturn(void);
	void OnBackspace(void);
	void OnDelete(void);
	void OnLeft(void);
	void OnRight(void);
private:
	//	перемещние строки редактирования
	void MoveEdit(int offset);
	//	обработка мышинного ввода
	bool ProcessMouseInput(VMouseState* pMS);
	//	обработка клавиатурного ввода
	bool ProcessKeyboardInput(VKeyboardState* pKS);
public:
	const char* Type(void) const {return "multiedit";}
};

#endif	//_MULTI_EDIT_H_