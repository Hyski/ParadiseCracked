/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   11.03.2001

************************************************************************/
#include "Precomp.h"
#include "Static.h"

Static::Static(const char *pName) : Widget(pName),m_ClickSample(0)
{
	Focus()->Enable(WidgetFocus::FF_KEYBOARD,false);
	m_LastEvent = LE_NONE;
	m_State = -1;
}

Static::~Static()
{
	Widget::SetImage(0);
	for(int i=0;i<m_pImages.size();i++)
		if(m_pImages[i]) m_pImages[i]->Release();
	if(m_ClickSample) m_ClickSample->Release();
}
//	добавить картинку (и соответственно состояние)
void Static::AddImage(WidgetImage *image)
{
	m_pImages.push_back(image);
}
//	установить виджет в определенное состояние
void Static::SetState(unsigned int state)
{
	if(state < m_pImages.size())
	{
		m_State = state;
		Widget::SetImage(m_pImages[state]);
	}
}
//	получить картинку по индексу
WidgetImage* Static::GetImage(unsigned int state) const
{
	if(state < m_pImages.size())
		return m_pImages[state];
	
	return 0;
}
//	обработка мышинного ввода
bool Static::ProcessMouseInput(VMouseState* pMS)
{
	m_LastEvent = LE_MOUSEMOVE;
	if(pMS->RButtonState)
		m_LastEvent = LE_RBUTTONPUSHED;
	if(pMS->LButtonState)
		m_LastEvent = LE_LBUTTONPUSHED;
	if(pMS->RButtonFront)
		m_LastEvent = LE_RBUTTONDOWN;
	if(pMS->RButtonBack)
		m_LastEvent = LE_RBUTTONUP;
	if(pMS->LButtonFront)
	{//	здесь мы произведем клик
		if(m_Player && !pMS->LDblClick) m_Player->Play(m_ClickSample);
		m_LastEvent = LE_LBUTTONDOWN;
	}
	if(pMS->LButtonBack)
		m_LastEvent = LE_LBUTTONUP;
	if(pMS->RDblClick)
		m_LastEvent = LE_RDBLCLICK;
	if(pMS->LDblClick)
		m_LastEvent = LE_LDBLCLICK;

	if(Parent()) Parent()->OnChange(this);

	m_LastEvent = LE_NONE;

	return false;
}
//	обработка потери мышиного ввода
bool Static::ProcessMouseLost(void)
{
	m_LastEvent = LE_MOUSELOST;

	if(Parent()) Parent()->OnChange(this);

	m_LastEvent = LE_NONE;

	return false;
}
//	обработка системного сообщения
void Static::OnSystemChange(void)
{
	WidgetImage* image = Widget::GetImage();

	Widget::SetImage(0);
	for(int i=0;i<m_pImages.size();i++)
		if(m_pImages[i]) m_pImages[i]->Reload();
	Widget::OnSystemChange();
	Widget::SetImage(image);
}
//	установить звук на клик
void Static::SetClickSample(Player::Sample* sample)
{
	if(m_ClickSample) m_ClickSample->Release();
	m_ClickSample = sample;
}
