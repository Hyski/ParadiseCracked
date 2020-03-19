/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   07.05.2001

************************************************************************/
#include "Precomp.h"
#include "Slide.h"

Slide::Slide(const char *pName) : Widget(pName),m_Value(0.2f),m_Step(0.01f)
{
	Insert(m_Image = new Widget("image"));
	m_Image->Insert(new Widget("image"));
}

Slide::~Slide()
{
}
//	установить значение нормированное к единице
void Slide::SetValue(float v)
{
	if(m_Step>0)
	{
		float fcontent = v/m_Step;
		int icontent = static_cast<int>(fcontent);
		if((fcontent-icontent)>0.5f) icontent++;
		m_Value = std::max(0.0f,std::min(icontent*m_Step,1.0f));
	}
	Update();
}
//	установить шаг
void Slide::SetStep(float s)
{
	m_Step = s;
	SetValue(m_Value);
}
//	установить картинку отображения
void Slide::SetImage(WidgetImage* pImage)
{
	m_Image->Child("image")->SetImage(pImage);
	m_Image->Child("image")->GetLocation()->Create(0,0,GetLocation()->Region()->Width(),GetLocation()->Region()->Height());
	Update();
}
//	обработка мышинного ввода
bool Slide::ProcessMouseInput(VMouseState* pMS)
{
	if(pMS->LButtonState)
	{
		SetValue((pMS->x-GetLocation()->OriginX()-GetLocation()->Region()->X())/GetLocation()->Region()->Width());
		if(Parent()) Parent()->OnChange(this);
	}

	if(Parent() && (Parent()->Focus()->Get(WidgetFocus::FF_KEYBOARD) != this) && (pMS->dx || pMS->dy))
			Parent()->Focus()->Set(WidgetFocus::FF_KEYBOARD,this);

	return false;
}
//	обработка клавиатурного ввода
bool Slide::ProcessKeyboardInput(VKeyboardState* /*pKS*/)
{
	return false;
}

void Slide::Update(void)
{
	m_Image->GetLocation()->Create(0,0,GetLocation()->Region()->Width()*m_Value,GetLocation()->Region()->Height());
}
