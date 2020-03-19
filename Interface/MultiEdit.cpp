/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   30.10.2001

************************************************************************/
#include "Precomp.h"
#include "../Common/FontMgr/LogicalText.h"
#include "../Common/FontMgr/Font2D.h"
#include "TextBox.h"
#include "Edit.h"
#include "MultiEdit.h"

//***********************************************************************//
//	class MultiEdit::Service
class MultiEdit::Service
{
public:
	//	получить цельную строку из разобранного текста
	static std::string GetString(const LogicalText& lt,unsigned int num,bool without_ns = false)
	{
		std::string result;

		if(num < lt.Text().size())
		{
			const LogicalText::LogicalString* str = &lt.Text()[num];
			
			int i;
			for(i=0;i<(str->m_vWords.size()-1);i++)
			{
				result += str->m_vWords[i]+" ";
			}
			result += str->m_vWords[i];
			if(str->m_EOS && !without_ns) result += "\n";
		}

		return result;
	}
	//	получить текст c заменой в разобранном тексте определенной строки
	static std::string ReplaceString(const LogicalText& lt,const char* new_str,unsigned int num)
	{
		std::string result;
		
		if(lt.Text().size())
		{
			if(num < lt.Text().size())
			{
				for(int i=0;i<lt.Text().size();i++)
				{
					if(i != num)
					{
						result += GetString(lt,i);
					}
					else
					{
						result += new_str;
					}
				}
			}
			else
			{
				result = lt.SourceText()+new_str;
			}

		}
		else
		{
			result = new_str;
		}

		return result;
	}
	//	удалить строку в разобранном тексте
	static std::string DeleteString(const LogicalText& lt,unsigned int num)
	{
		std::string result;
		
		if(lt.Text().size())
		{
			if(num < (lt.Text().size()))
			{
				for(int i=0;i<lt.Text().size();i++)
				{
					if(i != num) result += GetString(lt,i);
				}
			}
			else
			{
				result = lt.SourceText();
			}
		}

		return result;
	}
};

//***********************************************************************//
//	class MultiEdit::EditController
class MultiEdit::EditController : public Edit::Controller
{
private:
	MultiEdit* m_FeedBack;
public:
	EditController(MultiEdit* feed_back) : m_FeedBack(feed_back) {}
	~EditController() {}
public:
	void OnReturn(void) {m_FeedBack->OnReturn();}
	void OnBackspace(void) {m_FeedBack->OnBackspace();}
	void OnDelete(void) {m_FeedBack->OnDelete();}
	void OnLeft(void) {m_FeedBack->OnLeft();}
	void OnRight(void) {m_FeedBack->OnRight();}
};

//***********************************************************************//
//	class MultiEdit
MultiEdit::MultiEdit(const char* name) : Widget(name),m_EditOffset(0),m_EController(0)
{
	m_EController = new EditController(this);
	//	основные элементы отображения
	Insert(m_TextBox = new TextBox("text"));
	Insert(m_Edit = new Edit("edit"));
	//	устанавливаем выравнивание
	m_TextBox->Align(LogicalText::T_TOP|LogicalText::T_LEFT);
	m_Edit->Align(LogicalText::T_TOP|LogicalText::T_LEFT);
	m_Edit->SetController(m_EController);
	//	клавиатурный фокус находится на строке редактирования
	Focus()->Set(WidgetFocus::FF_KEYBOARD,m_Edit);
}

MultiEdit::~MultiEdit()
{
	delete m_EController;
}
//	установить размеры
void MultiEdit::SetRegion(int left,int top,int right,int bottom)
{
	GetLocation()->Create(left,top,right,bottom);
	m_TextBox->SetRegion(0,0,right-left,bottom-top);
	m_Edit->SetRegion(0,0,right-left,m_Edit->GetFont()->Height()+5);
	m_Edit->SetLength(1000);
}
//	установить шрифт
void MultiEdit::SetFont(const char* name,int color)
{
	m_TextBox->SetFont(name,color);
	m_Edit->SetFont(name,color);
}
//	установить свойства
void MultiEdit::SetTraits(const Traits& t)
{
	if(t.m_CaretImageName)
	{
		Edit::Traits et = {t.m_CaretImageName};
		m_Edit->SetTraits(et);
	}
}
//	очистить текст
void MultiEdit::Clear(void)
{
	m_TextBox->SetText("");
	m_Edit->SetText("");
	m_Edit->MoveTo(0,0);
	m_EditOffset = 0;
}
//	получить текст
const std::string& MultiEdit::GetText(void) const
{
	static std::string text;
	
	text = Service::ReplaceString(m_TextBox->GetLogicalText(),m_Edit->GetText().c_str(),m_EditOffset).c_str();

	return text;
}
//	обработка мышинного ввода
bool MultiEdit::ProcessMouseInput(VMouseState* pMS)
{
	if(Parent() && (Parent()->Focus()->Get(WidgetFocus::FF_KEYBOARD) != this) && (pMS->dx || pMS->dy))
			Parent()->Focus()->Set(WidgetFocus::FF_KEYBOARD,this);

	return true;
}
//	обработка клавиатурного ввода
bool MultiEdit::ProcessKeyboardInput(VKeyboardState* pKS)
{
	if(Input::KeyFront(DIK_UP|Input::RESET))
	{//	передвигаем строку редактирования вверх
		if(m_EditOffset)
		{
			MoveEdit(-1);
		}
	}
	if(Input::KeyFront(DIK_DOWN|Input::RESET))
	{//	передвигаем строку редактирования вниз
		if(m_EditOffset < (m_TextBox->GetLogicalText().Text().size()-1))
		{
			MoveEdit(1);
		}
	}

	return true;
}
//	перемещние строки редактирования
void MultiEdit::MoveEdit(int offset)
{
	std::string edit_text = m_Edit->GetText();
	int caret_pos = m_Edit->GetCaretPos();
	
	m_Edit->SetText(Service::GetString(m_TextBox->GetLogicalText(),m_EditOffset+offset,true).c_str());
	
	edit_text += "\n";
	m_TextBox->SetText(Service::ReplaceString(m_TextBox->GetLogicalText(),edit_text.c_str(),m_EditOffset).c_str());
	m_TextBox->SetText(Service::ReplaceString(m_TextBox->GetLogicalText(),"\n",m_EditOffset+offset).c_str());
	
	m_EditOffset += offset;
	m_Edit->Move(0,m_Edit->GetFont()->Height()*offset);
	m_Edit->SeekCaret(caret_pos,Edit::S_BEGIN);
}

void MultiEdit::OnReturn(void)
{
	if(((m_TextBox->GetLogicalText().Text().size()+1)*m_Edit->GetFont()->Height()) < GetLocation()->Region()->Height())
	{
		std::string edit_text = m_Edit->GetText();
		
		edit_text.insert(m_Edit->GetCaretPos(),"\n");
		edit_text += "\n";
		
		m_TextBox->SetText(Service::ReplaceString(m_TextBox->GetLogicalText(),edit_text.c_str(),m_EditOffset).c_str());
		
		m_Edit->SetText(Service::GetString(m_TextBox->GetLogicalText(),m_EditOffset+1,true).c_str());
		m_TextBox->SetText(Service::ReplaceString(m_TextBox->GetLogicalText(),"\n",m_EditOffset+1).c_str());
		
		m_EditOffset++;
		m_Edit->Move(0,m_Edit->GetFont()->Height());
	}
}

void MultiEdit::OnBackspace(void)
{
	if(m_EditOffset)
	{
		std::string two_string = Service::GetString(m_TextBox->GetLogicalText(),m_EditOffset-1,true);
		int caret_pos = two_string.size();
		two_string += m_Edit->GetText();

		//	проверка на вместимость двух строк
		int width,height;
		m_Edit->GetFont()->GetSize(two_string.c_str(),&width,&height);
		if(width<GetLocation()->Region()->Width())
		{
			m_TextBox->SetText(Service::ReplaceString(m_TextBox->GetLogicalText(),"\n",m_EditOffset-1).c_str());
			m_TextBox->SetText(Service::DeleteString(m_TextBox->GetLogicalText(),m_EditOffset).c_str());

			m_Edit->SetText(two_string.c_str());
			m_EditOffset--;
			m_Edit->Move(0,-m_Edit->GetFont()->Height());
			m_Edit->SeekCaret(caret_pos,Edit::S_BEGIN);
		}
	}
}

void MultiEdit::OnDelete(void)
{
	if(m_EditOffset < (m_TextBox->GetLogicalText().Text().size()-1))
	{
		std::string two_string = m_Edit->GetText();
		int caret_pos = two_string.size();
		two_string += Service::GetString(m_TextBox->GetLogicalText(),m_EditOffset+1,true);
		//	проверка на вместимость двух строк
		int width,height;
		m_Edit->GetFont()->GetSize(two_string.c_str(),&width,&height);
		if(width<GetLocation()->Region()->Width())
		{
			m_TextBox->SetText(Service::DeleteString(m_TextBox->GetLogicalText(),m_EditOffset+1).c_str());
			m_Edit->SetText(two_string.c_str());
			m_Edit->SeekCaret(caret_pos,Edit::S_BEGIN);
		}
	}
}

void MultiEdit::OnLeft(void)
{
	if(m_EditOffset)
	{
		MoveEdit(-1);
		m_Edit->SeekCaret(0,Edit::S_END);
	}
}

void MultiEdit::OnRight(void)
{
	if(m_EditOffset < (m_TextBox->GetLogicalText().Text().size()-1))
	{
		MoveEdit(1);
		m_Edit->SeekCaret(0,Edit::S_BEGIN);
	}
}

