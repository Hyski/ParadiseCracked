/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   26.03.2001

************************************************************************/
#include "Precomp.h"
#include "../Common/D3DApp/Input/CharKeySupport.h"
#include "../Common/FontMgr/FontMgr.h"
#include "../Common/FontMgr/Font2D.h"
#include "DXImageMgr.h"
#include "Edit.h"

Edit::Edit(const char *pName) : Text(pName),m_Controller(0)
{
	Focus()->Enable(WidgetFocus::FF_KEYBOARD,true);
	//	-----------------------------------
	Insert(m_Caret = new Caret());
	m_InsertMode = true;
}

Edit::~Edit()
{
}
//	установить контроллер
void Edit::SetController(Controller* controller)
{
	m_Controller = controller;
}
//	проинициализировать параметры
void Edit::SetTraits(const Traits& t)
{
	m_Caret->SetImage(DXImageMgr::Instance()->CreateImage(t.m_CaretImageName));
}
//	задать максимальное кол-во символов
void Edit::SetLength(int symbols)
{
	m_Caret->SetMaxSym(symbols);
}

//	задать прямоугольник вывода текста
void Edit::SetRegion(int left,int top,int right,int bottom)
{
	Text::SetRegion(left,top,right,bottom);
}
//	установить шрифт
void Edit::SetFont(const char* name,int color)
{
	int width,height;

	Text::SetFont(name,color);
	//	--------------------------------------
	FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize(" ",&width,&height);
	m_Caret->SetMovingLine(FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->Height(),width);
}
//	устнановить текст
void Edit::SetText(const char* text)
{
	Text::SetText(text);
	//	передвигаем каретку в начало
	m_Caret->SetPos(0);
	MoveCaret(-m_Caret->GetLocation()->Region()->X(),m_Caret->SpaceWidth());
}
//	выровнять текст
void Edit::Align(int align)
{
	align &= (~LogicalText::T_RIGHT)&(~LogicalText::T_WIDE)&(~LogicalText::T_HCENTER);
	Text::Align(align);
}
//	установить режим ввода текста
void Edit::SetInsertMode(bool insert_mode)
{
	m_InsertMode = insert_mode;
}
//	переместить курсор в определенную позицию
//	enum SEEK {S_BEGIN,S_CURRENT,S_END};
void Edit::SeekCaret(int pos,SEEK seek)
{
	switch(seek)
	{
	case S_CURRENT:
		pos = m_Caret->Pos() + pos;
		break;
	case S_END:
		pos = m_Text.SourceText().size() - pos;
		break;
	}

	MoveCaretTo(pos);
}
//	переместить каретку в заданную позицию с проверкой
void Edit::	MoveCaretTo(int pos)
{
	if(m_Text.SourceText().size())
	{
		pos = std::max(0,std::min(pos,static_cast<int>(m_Text.SourceText().size())));
		m_Caret->SetPos(pos);

		char* buffer = new char[pos+1];
		int width,height;
		int next_width;

		strncpy(buffer,m_Text.SourceText().c_str(),pos);
		buffer[pos] = 0;
		FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize(buffer,&width,&height);
		FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize((m_Caret->Pos()<m_Text.SourceText().size())?m_Text.SourceText()[m_Caret->Pos()]:' ',&next_width,&height);

		m_Caret->MoveTo(width,m_Caret->GetLocation()->Region()->Y());
		m_Caret->SetNewWidth(next_width);

		delete buffer;
	}
}
//	получить позицию курсора в символах
unsigned int Edit::GetCaretPos(void)
{
	return m_Caret->Pos();
}
//	смена видео режима
void Edit::OnSystemChange(void)
{
	Text::OnSystemChange();
	m_Caret->OnSystemChange();
}
//	обработка мышинного ввода
bool Edit::ProcessMouseInput(VMouseState* pMS)
{
	if(Parent() && (Parent()->Focus()->Get(WidgetFocus::FF_KEYBOARD) != this) && (pMS->dx || pMS->dy))
	{
		m_Caret->Activate(true);
		Parent()->Focus()->Set(WidgetFocus::FF_KEYBOARD,this);
	}

	return false;
}
//	обработка клавиатурного ввода
bool Edit::ProcessKeyboardInput(VKeyboardState* /*pKS*/)
{
	static unsigned char ch;
	static int width,height;
	static int text_width,text_height;
	static int next_width,next_height;

	//	покажем курсор если он не виден
	if(!m_Caret->IsActivated()) m_Caret->Activate(true);
	//	обработаем ввод
	if(Input::KeyBack(DIK_RETURN|Input::RESET) || Input::KeyBack(DIK_NUMPADENTER|Input::RESET))
	{
		if(Parent()) Parent()->OnChange(this);
		if(m_Controller) m_Controller->OnReturn();
	}
	if(Input::KeyBack(DIK_HOME|Input::RESET))
	{//	передвигаем каретку в начало
		m_Caret->SetPos(0);
		MoveCaret(-m_Caret->GetLocation()->Region()->X(),m_Caret->SpaceWidth());
	}
	if(Input::KeyBack(DIK_END|Input::RESET))
	{//	передвигаем каретку за последний символ
		FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize(m_Text.SourceText().c_str(),&text_width,&text_height);
		m_Caret->SetPos(m_Text.SourceText().size());
		m_Caret->MoveTo(0,text_height);
		MoveCaret(text_width,m_Caret->SpaceWidth());
	}
	if(Input::KeyBack(DIK_LEFT|Input::RESET))
	{//	передвигаем каретку влево
		if(m_Caret->Reverse())
		{
			FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize(m_Text.SourceText()[m_Caret->Pos()],&width,&height);
			MoveCaret(-width,width);
		}
		else
		{
			if(m_Controller) m_Controller->OnLeft();
		}
	}
	if(Input::KeyBack(DIK_RIGHT|Input::RESET))
	{//	передвигаем каретку вправо
		if((m_Caret->Pos()<m_Text.SourceText().size()) && m_Caret->Forward())
		{
			FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize(m_Text.SourceText()[m_Caret->Pos()-1],&width,&height);
			FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize((m_Caret->Pos()<m_Text.SourceText().size())?m_Text.SourceText()[m_Caret->Pos()]:' ',&next_width,&next_height);
			MoveCaret(width,next_width);
		}
		else
		{
			if(m_Controller) m_Controller->OnRight();
		}
	}
	if(Input::KeyBack(DIK_BACKSPACE|Input::RESET))
	{//	передвигаем каретку влево с уничтожением символа
		if(m_Caret->Reverse())
		{
			FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize(m_Text.SourceText()[m_Caret->Pos()],&width,&height);
			FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize(((m_Caret->Pos()+1)<m_Text.SourceText().size())?m_Text.SourceText()[m_Caret->Pos()+1]:' ',&next_width,&next_height);
			DeleteCharFromText(m_Caret->Pos());
			MoveCaret(-width,next_width);
		}
		else
		{
			if(m_Controller) m_Controller->OnBackspace();
		}
	}	//	проверяем на ввод символов
	if(Input::KeyBack(DIK_DELETE|Input::RESET))
	{//	уничтожением символ под кареткой
		if(m_Caret->Pos()<m_Text.SourceText().size())
		{
			DeleteCharFromText(m_Caret->Pos());
			FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize(((m_Caret->Pos())<m_Text.SourceText().size())?m_Text.SourceText()[m_Caret->Pos()]:' ',&next_width,&next_height);
			MoveCaret(0,next_width);
		}
		else
		{
			if(m_Controller) m_Controller->OnDelete();
		}
	}	//	проверяем на ввод символов
	if((ch = CharKeySupport::ScanKeyboard()) && m_FontName.size())
	{
		//	могут возникнуть две ситуации: каретка за последним символом и каретка в середине текста
		//	существуют два режима вставки символа: вставить символ и заменить текущий на новый символ
		//	за это отвечает флаг m_InsertMode.
		//	высчитываем необходимые размеры
		FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize(ch,&width,&height);
		FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize(m_Text.SourceText().c_str(),&text_width,&text_height);
		text_width += width;
		if(!m_InsertMode)
		{
			if(m_Caret->Pos()<m_Text.SourceText().size())
				FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize(m_Text.SourceText()[m_Caret->Pos()],&next_width,&next_height);
			else
				next_width = 0;
			text_width -= next_width;
		}
		//	проверяем на вместимость
		if((text_width+m_Caret->SpaceWidth())<GetLocation()->Region()->Width())
		{
			if(m_Caret->Forward())
			{//	передвинули логический курсор
				//	теперь необходимо вставить символ в текст
				InsertCharToText(ch,m_Caret->Pos()-1);
				//	передвинуть экранный курсор
				FontMgr::Instance()->GetFont2D(m_FontName.c_str(),m_FontColor)->GetSize((m_Caret->Pos()<m_Text.SourceText().size())?m_Text.SourceText()[m_Caret->Pos()]:' ',&next_width,&next_height);
				MoveCaret(width,next_width);
			}
		}
	}

	return false;
}
//	обработка потери клавиатурного ввода
bool Edit::ProcessKeyboardLost(void)
{
	m_Caret->Activate(false);

	return false;
}
//	вставить символ в текст
void Edit::InsertCharToText(unsigned char ch,int pos)
{
	char *pBuff = new char[m_Text.SourceText().size()+2];

	//	копируем строку
	strcpy(pBuff,m_Text.SourceText().c_str());
	//	последний нуль
	pBuff[m_Text.SourceText().size()+1] = '\0';
	//	перемещаем кусок строки
	if(m_InsertMode && (pos<m_Text.SourceText().size()))
		strcpy(pBuff+pos+1,m_Text.SourceText().c_str()+pos);
	//	пишем символ
	pBuff[pos] = ch;
	//	рассчитывем текст
	Text::SetText(pBuff);
	//	удаляем буфер преобразований
	delete[] pBuff;
}
//	удалить символ из текста
void Edit::DeleteCharFromText(int pos)
{
	char *pBuff = new char[m_Text.SourceText().size()+1];

	//	копируем строку
	strcpy(pBuff,m_Text.SourceText().c_str());
	//	перемещаем кусок строки
	if(pos<m_Text.SourceText().size())
		strcpy(pBuff+pos,m_Text.SourceText().c_str()+pos+1);
	//	рассчитывем текст
	Text::SetText(pBuff);
	//	удаляем буфер преобразований
	delete[] pBuff;
}
//	передвинуть курсор
void Edit::MoveCaret(int dx,int new_width)
{
	m_Caret->Move(dx,0);
	m_Caret->SetNewWidth(new_width);
}
//***********************************************************************//
//**********************    class Edit::Caret     ***********************//
const float Edit::Caret::m_CaretBlinkFreq = 0.3f;	//	частота мигания курсора
float Edit::Caret::m_BlinkSeconds = 0;			//	текущее положение секунд
const int Edit::Caret::m_CaretHeight = 2;		//	высота курсора в пикселах
Edit::Caret::Caret() : Widget("caret")
{
	m_SymMax = 0;
	m_SymPos = 0;
	m_SpaceWidth = 0;
	//	------------------------------
	m_bActivated = false;
	SetVisible(false);
}

Edit::Caret::~Caret()
{
}
//	установить размеры курсора
void Edit::Caret::SetMovingLine(int top,int space_width)
{
	m_SpaceWidth = space_width;
	GetLocation()->Create(0,top,space_width,top+m_CaretHeight);
}
//	установить максимальное кол-во символов
void Edit::Caret::SetMaxSym(int symbols)
{
	m_SymMax = symbols;
	m_SymPos = std::min(m_SymPos,m_SymMax);
}
//	переместить каретку назад
bool Edit::Caret::Reverse(void)
{
	if(m_SymPos>0)
	{
		m_SymPos--;
		return true;
	}
	return false;
}
//	переместить каретку вперед
bool Edit::Caret::Forward(void)
{
	if(m_SymPos<m_SymMax)
	{
		m_SymPos++;
		return true;
	}
	return false;
}
//	переместить каретку
void Edit::Caret::SetPos(unsigned int pos)
{
	m_SymPos = std::min(m_SymMax,(int)pos);
}
//	установить новую ширину каретки
void Edit::Caret::SetNewWidth(int width)
{
	GetLocation()->Create(GetLocation()->Region()->X(),
					   GetLocation()->Region()->Y(),
					   GetLocation()->Region()->X()+width,
					   GetLocation()->Region()->Y()+m_CaretHeight);
}
//	установить видимость курсора
void Edit::Caret::Activate(bool bActivate)
{
	m_bActivated = bActivate;
	SetVisible(m_bActivated);
}
//	подмигивание курсора
bool Edit::Caret::Tick(void)
{
	if((Timer::GetSeconds()-m_BlinkSeconds)>m_CaretBlinkFreq)
	{
		m_BlinkSeconds = Timer::GetSeconds();
		if(m_bActivated)
			SetVisible(!IsVisible());
	}

	return false;
}
