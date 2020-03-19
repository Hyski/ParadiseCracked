/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   05.03.2001

************************************************************************/
#include "Precomp.h"
#include "../Common/DataMgr/DataMgr.h"
#include "../Common/DataMgr/TxtFile.h"
#include "../Common/UI/Widget.h"
#include "../Common/UI/Button.h"
#include "../Common/UI/Static.h"
#include "../Common/UI/Slide.h"
#include "Text.h"
#include "TextBox.h"
#include "Edit.h"
#include "MultiEdit.h"
#include "Dialog.h"
#include "DXImageMgr.h"
#include "WidgetSoundMgr.h"
#include "WidgetFactory.h"
//	дл€ строковых ресурсов
#include "../Logic2/logicdefs.h"
#include "../Logic2/DirtyLinks.h"

//	типы известных виджетов
const char *g_pWidget		= "widget";
const char *g_pButton		= "button";
const char *g_pStatic		= "static";
const char *g_pText			= "text";
const char *g_pTextBox		= "textbox";
const char *g_pEdit			= "edit";
const char *g_pSlider		= "slider";
const char *g_pMultiEdit	= "multiedit";
const char *g_pDialog		= "dialog";
//	токены скрипта
const char *g_pName		= "name";
const char *g_pType		= "type";
const char *g_pRegion	= "region";
const char *g_pLink		= "link";
const char *g_pImage	= "image";
const char *g_pTip		= "tip";
//	дл€ кнопки
const char *g_pButtonImageNormal	= "button_image_normal";
const char *g_pButtonImageSelected	= "button_image_selected";
const char *g_pButtonImagePushed	= "button_image_pushed";
const char *g_pButtonSoundNormal	= "button_sound_normal";
const char *g_pButtonSoundSelected	= "button_sound_selected";
const char *g_pButtonSoundPushed	= "button_sound_pushed";
//	дл€ статика
const char *g_pStaticStates			= "static_states";
const char *g_pStaticImage			= "static_image";
const char *g_pStaticSoundClick		= "static_sound_click";

//	дл€ статического текста
const char *g_pTextFont				= "text_font";
const char *g_pTextColor			= "text_color";
const char *g_pTextText				= "text_text";
const char *g_pTextAlign			= "text_align";
struct 
{
	const char *m_pAlign;
	LogicalText::TEXT_ALIGN m_ltAlign;
} const g_pAlign[7] = 
{
	{"top",		LogicalText::T_TOP},
	{"bottom",	LogicalText::T_BOTTOM},
	{"left",	LogicalText::T_LEFT},
	{"right",	LogicalText::T_RIGHT},
	{"hcenter",	LogicalText::T_HCENTER},
	{"vcenter",	LogicalText::T_VCENTER},
	{"wide",	LogicalText::T_WIDE}
};
//	дл€ бокса с текстом
const char *g_pTextBoxFont				= "textbox_font";
const char *g_pTextBoxColor				= "textbox_color";
const char *g_pTextBoxText				= "textbox_text";
const char *g_pTextBoxAlign				= "textbox_align";
const char *g_pTextBoxScrollRequire		= "textbox_scrollrequire";
const char *g_pTextBoxScrollUpImage		= "textbox_scrollupimage";
const char *g_pTextBoxScrollDownImage	= "textbox_scrolldownimage";
const char *g_pTextBoxScrollWidth		= "textbox_scrollwidth";
const char *g_pTextBoxScrollHeight		= "textbox_scrollheight";
const char *g_pTextBoxMode				= "textbox_mode";
struct 
{
	const char* m_Text;
	TextBox::MODE m_Mode;
} const g_tbMode[2] = 
{
	{"normal",	TextBox::M_NORMAL},
	{"console",	TextBox::M_CONSOLE}
};
//	дл€ строки редактировани€
const char *g_pEditFont				= "edit_font";
const char *g_pEditColor			= "edit_color";
const char *g_pEditText				= "edit_text";
const char *g_pEditLength			= "edit_length";
const char *g_pEditCaret			= "edit_caret";
const char *g_pEditInsertMode		= "edit_insertmode";
//	дл€ slide'а
const char *g_pSlideStep			= "slider_step";
//	дл€ текстового процессора
const char *g_pMultiEditFont		= "multiedit_font";
const char *g_pMultiEditColor		= "multiedit_color";
const char *g_pMultiEditText		= "multiedit_text";
const char *g_pMultiEditCaret		= "multiedit_caret";

WidgetFactory::Deleter WidgetFactory::m_Deleter;
WidgetFactory::WidgetFactory() : m_pScriptPath("scripts/interface/"),m_pScriptExt(".txt")
{
}

WidgetFactory::~WidgetFactory()
{
}

void WidgetFactory::LoadScript(Dialog* pOwner,const char* pScript)
{
	std::string sScript = m_pScriptPath;
	TxtFile script;
	unsigned int name,type;
	bool bError = false;

	sScript += pScript;
	sScript += m_pScriptExt;

	m_OriginX = 0;
	m_OriginY = 0;

	//	читаем файл скрипта
	if(script.Load(DataMgr::Load(sScript.c_str())))
	{
		//	освобождаем ресурсы
		DataMgr::Release(sScript.c_str());
		//	считываем скрипт
		if(script.FindInRow(g_pName,0,&name) && script.FindInRow(g_pType,0,&type))
		{
			for(int i=1;i<script.SizeY();i++)
			{//	считываем информацию
				if(i == 1)
				{//	перва€ строчка идет как описание родительского виджета
					pOwner->GetLocation()->Create(GetRegion(&script,i));
					pOwner->SetImage(GetImage(g_pImage,&script,i));
					m_OriginX = pOwner->GetLocation()->Region()->X();
					m_OriginY = pOwner->GetLocation()->Region()->Y();
				}
				else
				{//	дочерние элементы
					switch(WidgetType(script(i,type).c_str()))
					{
					case WT_WIDGET:
						if(!pOwner->Insert(LoadWidget(script(i,name).c_str(),&script,i),(Dialog::LINK)GetLink(&script,i))) bError = true;
						break;
					case WT_BUTTON:
						if(!pOwner->Insert(LoadButton(script(i,name).c_str(),&script,i),(Dialog::LINK)GetLink(&script,i))) bError = true;
						break;
					case WT_STATIC:
						if(!pOwner->Insert(LoadStatic(script(i,name).c_str(),&script,i),(Dialog::LINK)GetLink(&script,i))) bError = true;
						break;
					case WT_TEXT:
						if(!pOwner->Insert(LoadText(script(i,name).c_str(),&script,i),(Dialog::LINK)GetLink(&script,i))) bError = true;
						break;
					case WT_TEXTBOX:
						if(!pOwner->Insert(LoadTextBox(script(i,name).c_str(),&script,i),(Dialog::LINK)GetLink(&script,i))) bError = true;
						break;
					case WT_EDIT:
						if(!pOwner->Insert(LoadEdit(script(i,name).c_str(),&script,i),(Dialog::LINK)GetLink(&script,i))) bError = true;
						break;
					case WT_SLIDER:
						if(!pOwner->Insert(LoadSlider(script(i,name).c_str(),&script,i),(Dialog::LINK)GetLink(&script,i))) bError = true;
						break;
					case WT_MULTIEDIT:
						if(!pOwner->Insert(LoadMultiEdit(script(i,name).c_str(),&script,i),(Dialog::LINK)GetLink(&script,i))) bError = true;
						break;
					}
				}
				if(bError)
					throw CASUS("ќшибка при загрузке скрипта.");

			}
		}
	}
}

//*********************************************************************************//
WidgetFactory::WIDGET_TYPE WidgetFactory::WidgetType(const char *pType)
{
	if(!strcmp(pType,g_pWidget))
		return WT_WIDGET;
	if(!strcmp(pType,g_pButton))
		return WT_BUTTON;
	if(!strcmp(pType,g_pStatic))
		return WT_STATIC;
	if(!strcmp(pType,g_pText))
		return WT_TEXT;
	if(!strcmp(pType,g_pTextBox))
		return WT_TEXTBOX;
	if(!strcmp(pType,g_pEdit))
		return WT_EDIT;
	if(!strcmp(pType,g_pSlider))
		return WT_SLIDER;
	if(!strcmp(pType,g_pMultiEdit))
		return WT_MULTIEDIT;
	if(!strcmp(pType,g_pDialog))
		return WT_DIALOG;

	return WT_UNKNOWN;
}
//	возвращает тип виджета
WidgetFactory::WIDGET_TYPE WidgetFactory::WidgetType(Widget* widget)
{
	return WidgetType(widget->Type());
}

Widget* WidgetFactory::Assert(Widget *pOwner,const char *pName)
{
	Widget *pChild = pOwner->Child(pName);
	if(pChild) return pChild;
	else throw CASUS(std::string(std::string("Ёлемент интерфейса <")+std::string(pName)+std::string("> отсутсвует в <")+std::string(pOwner->Name())+">."));
}
//*********************************************************************************//
const VRegion& WidgetFactory::GetRegion(TxtFile* pScript,int iString)
{
	static VRegion r;
	VPoint *pPoints;
	VPoint pt;
	std::string sRegion;
	std::vector<VPoint> vPoints;
	char *pBuff;
	unsigned int x;
	int count;

	r.Release();

	if(pScript->FindInRow(g_pRegion,0,&x))
	{
		sRegion = (*pScript)(iString,x);
		pBuff = (char *)sRegion.c_str();
		while(pBuff)
		{
			count = sscanf(pBuff,"%d,%d",&pt.x,&pt.y);
			if(count != 2) break;
			vPoints.push_back(pt);
			pBuff = strchr(pBuff,',')+1;
			pBuff = strchr(pBuff,',');
			if(pBuff)
				pBuff++;
		}	
		if(vPoints.size())
		{
			pPoints = new VPoint[vPoints.size()];
			for(x=0;x<vPoints.size();x++)
				pPoints[x] = vPoints[x];
			if(vPoints.size() == 2)
				r.Create(pPoints[0].x,pPoints[0].y,pPoints[1].x,pPoints[1].y);
			else
				r.Create(pPoints,vPoints.size());
			delete[] pPoints;
		}
	}

	r.Transform(1,0,-m_OriginX,0,1,-m_OriginY);

	return r;
}

DXImage* WidgetFactory::GetImage(const char *pKey,TxtFile* pScript,int iString)
{
	std::string sImage;
	unsigned int x;

	if(pScript->FindInRow(pKey,0,&x))
	{
		sImage = (*pScript)(iString,x);
		return DXImageMgr::Instance()->CreateImage(sImage.c_str());
	}

	return 0;
}

Widget::Player::Sample* WidgetFactory::GetSound(const char *pKey,TxtFile* pScript,int iString)
{
	std::string sSound;
	unsigned int x;

	if(pScript->FindInRow(pKey,0,&x))
	{
		if(x < pScript->SizeX(iString))
		{
			sSound = (*pScript)(iString,x);
			if(sSound.size()) return WidgetSoundMgr::Instance()->CreateSample(sSound.c_str());
		}
	}

	return 0;
}

int WidgetFactory::GetLink(TxtFile* pScript,int iString)
{
	std::string link;
	unsigned int x;

	if(pScript->FindInRow(g_pLink,0,&x))
	{
		link = (*pScript)(iString,x);
		if(link == "left_top")		return Dialog::L_LEFT_TOP;
		if(link == "left_bottom")	return Dialog::L_LEFT_BOTTOM;
		if(link == "right_top")		return Dialog::L_RIGHT_TOP;
		if(link == "right_bottom")	return Dialog::L_RIGHT_BOTTOM;
		if(link == "center")		return Dialog::L_CENTER;
	}

	return Dialog::L_NONE;
}

void WidgetFactory::SetTip(Widget* pWidget,TxtFile* pScript,int iString)
{
	unsigned int x;
	
	if(pScript->FindInRow(g_pTip,0,&x))
	{
		std::string tip_label = (*pScript)(iString,x);

		if(tip_label.size())
		{
			pWidget->m_Tip.m_Text = DirtyLinks::GetStrRes(tip_label);
			pWidget->m_Tip.m_Activated = true;
		}
	}
}

//*********************************************************************************//
Widget* WidgetFactory::LoadWidget(const char *pName,TxtFile* pScript,int iString)
{
	Widget *pWidget;

	pWidget = new Widget(pName);
	pWidget->GetLocation()->Create(GetRegion(pScript,iString));
	pWidget->SetImage(GetImage(g_pImage,pScript,iString));
	pWidget->Enable(false);

	SetTip(pWidget,pScript,iString);

	return pWidget;
}

Widget* WidgetFactory::LoadButton(const char *pName,TxtFile* pScript,int iString)
{
	Button *pButton;

	pButton = new Button(pName);
	pButton->GetLocation()->Create(GetRegion(pScript,iString));
	//	устанавливаем картинки
	pButton->SetImage(Button::BS_NORMAL,GetImage(g_pButtonImageNormal,pScript,iString));
	pButton->SetImage(Button::BS_SELECTED,GetImage(g_pButtonImageSelected,pScript,iString));
	pButton->SetImage(Button::BS_PUSHED,GetImage(g_pButtonImagePushed,pScript,iString));
	//	устанавливаем звуки
	pButton->SetSample(Button::BS_NORMAL,GetSound(g_pButtonSoundNormal,pScript,iString));
	pButton->SetSample(Button::BS_SELECTED,GetSound(g_pButtonSoundSelected,pScript,iString));
	pButton->SetSample(Button::BS_PUSHED,GetSound(g_pButtonSoundPushed,pScript,iString));
	//	устанавливаем проигрыватель звуков
	pButton->SetPlayer(WidgetSoundMgr::Instance()->GetPlayer());

	SetTip(pButton,pScript,iString);

	return pButton;
}

Widget* WidgetFactory::LoadStatic(const char *pName,TxtFile* pScript,int iString)
{
	Static *pStatic;
	std::string content;
	char pImage[100];
	unsigned int x;
	int states;

	pStatic = new Static(pName);
	pStatic->GetLocation()->Create(GetRegion(pScript,iString));
	if(pScript->FindInRow(g_pStaticStates,0,&x))
	{
		states = atoi((*pScript)(iString,x).c_str());
		if(states)
		{
			for(int i=0;i<states;i++)
			{
				sprintf(pImage,"%s%d",g_pStaticImage,i);
				pStatic->AddImage(GetImage(pImage,pScript,iString));
			}
			pStatic->SetState(0);
		}
	}
	//	устанавливаем звук
	pStatic->SetClickSample(GetSound(g_pStaticSoundClick,pScript,iString));
	//	устанавливаем проигрыватель звуков
	pStatic->SetPlayer(WidgetSoundMgr::Instance()->GetPlayer());

	SetTip(pStatic,pScript,iString);

	return pStatic;
}

Widget* WidgetFactory::LoadText(const char *pName,TxtFile* pScript,int iString)
{
	Text *pText;
	unsigned int x,y,align;
	char *pAlign;
	int left,top,right,bottom;
	bool bFind;
	
	pText = new Text(pName);
	GetRegion(pScript,iString).GetRect(&left,&top,&right,&bottom);
	pText->SetRegion(left,top,right,bottom);
	if(pScript->FindInRow(g_pTextFont,0,&x) && pScript->FindInRow(g_pTextColor,0,&y))
	{
		pText->SetFont((*pScript)(iString,x).c_str(),atoi((*pScript)(iString,y).c_str()));
	}
	if(pScript->FindInRow(g_pTextAlign,0,&x))
	{//	разбираем ключи выравнивани€ текста
		align = 0;
		pAlign = (char *)(*pScript)(iString,x).c_str();
		y = (sizeof(g_pAlign)/sizeof(g_pAlign[0]));
		while(pAlign && *pAlign)
		{
			bFind = false;
			for(int i=0;i<y;i++)
			{
				if(!strncmp(pAlign,g_pAlign[i].m_pAlign,strlen(g_pAlign[i].m_pAlign)))
				{
					align |= g_pAlign[i].m_ltAlign;
					pAlign += strlen(g_pAlign[i].m_pAlign);
					if(*pAlign) pAlign++;
					bFind = true;
					break;
				}
			}
			if(bFind) continue;
			else break;
		}
		pText->Align(align);
	}
	if(pScript->FindInRow(g_pTextText,0,&x))
	{
		pText->SetText((*pScript)(iString,x).c_str());
	}

	SetTip(pText,pScript,iString);

	return pText;
}

Widget* WidgetFactory::LoadTextBox(const char *pName,TxtFile* pScript,int iString)
{
	TextBox *pTextBox;
	unsigned int x,y,align;
	char *pAlign;
	int left,top,right,bottom;
	bool bFind;

	pTextBox = new TextBox(pName);
	GetRegion(pScript,iString).GetRect(&left,&top,&right,&bottom);
	pTextBox->SetRegion(left,top,right,bottom);
	if(pScript->FindInRow(g_pTextBoxFont,0,&x) && pScript->FindInRow(g_pTextBoxColor,0,&y))
	{
		pTextBox->SetFont((*pScript)(iString,x).c_str(),atoi((*pScript)(iString,y).c_str()));
	}
	if(pScript->FindInRow(g_pTextBoxAlign,0,&x))
	{//	разбираем ключи выравнивани€ текста
		align = 0;
		pAlign = (char *)(*pScript)(iString,x).c_str();
		y = (sizeof(g_pAlign)/sizeof(g_pAlign[0]));
		while(pAlign && *pAlign)
		{
			bFind = false;
			for(int i=0;i<y;i++)
			{
				if(!strncmp(pAlign,g_pAlign[i].m_pAlign,strlen(g_pAlign[i].m_pAlign)))
				{
					align |= g_pAlign[i].m_ltAlign;
					pAlign += strlen(g_pAlign[i].m_pAlign);
					if(*pAlign) pAlign++;
					bFind = true;
					break;
				}
			}
			if(bFind) continue;
			else break;
		}
		pTextBox->Align(align);
	}
	if(pScript->FindInRow(g_pTextBoxText,0,&x) && (*pScript)(iString,x).size())
	{
		pTextBox->SetText((*pScript)(iString,x).c_str());
	}
	if(pScript->FindInRow(g_pTextBoxScrollRequire,0,&x) && atoi((*pScript)(iString,x).c_str()))
	{
		TextBox::ScrollTraits st;
		
		if(pScript->FindInRow(g_pTextBoxScrollUpImage,0,&x))
			st.m_UpImageName = (*pScript)(iString,x).c_str();
		if(pScript->FindInRow(g_pTextBoxScrollDownImage,0,&x))
			st.m_DownImageName = (*pScript)(iString,x).c_str();
		if(pScript->FindInRow(g_pTextBoxScrollWidth,0,&x))
			st.m_Width = atoi((*pScript)(iString,x).c_str());
		if(pScript->FindInRow(g_pTextBoxScrollHeight,0,&x))
			st.m_Height = atoi((*pScript)(iString,x).c_str());
		pTextBox->SetScroll(st);
	}
	//	узнаем режим вывода текста 
	if(pScript->FindInRow(g_pTextBoxMode,0,&x) && (x<pScript->SizeX(iString)))
	{
		for(int i=0;i<2;i++)
		{
			if((*pScript)(iString,x) == g_tbMode[i].m_Text)
			{
				pTextBox->SetMode(g_tbMode[i].m_Mode);
				break;
			}
		}
	}

	SetTip(pTextBox,pScript,iString);

	return pTextBox;
}

Widget* WidgetFactory::LoadEdit(const char *pName,TxtFile* pScript,int iString)
{
	Edit *pEdit;
	unsigned int x,y;
	int left,top,right,bottom;

	pEdit = new Edit(pName);
	GetRegion(pScript,iString).GetRect(&left,&top,&right,&bottom);
	pEdit->SetRegion(left,top,right,bottom);
	//	установка шрифта
	if(pScript->FindInRow(g_pEditFont,0,&x) && pScript->FindInRow(g_pEditColor,0,&y))
	{
		pEdit->SetFont((*pScript)(iString,x).c_str(),atoi((*pScript)(iString,y).c_str()));
	}
	//	установка свойств
	if(pScript->FindInRow(g_pEditCaret,0,&x))
	{
		Edit::Traits tr;

		tr.m_CaretImageName = (*pScript)(iString,x).c_str();
		pEdit->SetTraits(tr);
	}
	//	установка режима ввода
	if(pScript->FindInRow(g_pEditInsertMode,0,&x))
	{
		pEdit->SetInsertMode(atoi((*pScript)(iString,x).c_str()) != 0);
	}
	//	установка максимальной длины в символах
	if(pScript->FindInRow(g_pEditLength,0,&x))
	{
		pEdit->SetLength(atoi((*pScript)(iString,x).c_str()));
	}
	//	установка текста
	if(pScript->FindInRow(g_pEditText,0,&x) && (*pScript)(iString,x).size())
	{
		pEdit->SetText((*pScript)(iString,x).c_str());
	}

	SetTip(pEdit,pScript,iString);

	return pEdit;
}

Widget* WidgetFactory::LoadSlider(const char *pName,TxtFile* pScript,int iString)
{
	Slide* pSlide;
	unsigned int x;

	pSlide = new Slide(pName);
	pSlide->GetLocation()->Create(GetRegion(pScript,iString));
	pSlide->SetImage(GetImage(g_pImage,pScript,iString));
	//	установка шага
	if(pScript->FindInRow(g_pSlideStep,0,&x))
	{
		pSlide->SetStep(atof((*pScript)(iString,x).c_str()));
	}

	SetTip(pSlide,pScript,iString);

	return pSlide;
}

Widget* WidgetFactory::LoadMultiEdit(const char *pName,TxtFile* pScript,int iString)
{
	MultiEdit* pMultiEdit;
	unsigned int x,y;
	int left,top,right,bottom;

	pMultiEdit = new MultiEdit(pName);

	//	установка шрифта
	if(pScript->FindInRow(g_pMultiEditFont,0,&x) && pScript->FindInRow(g_pMultiEditColor,0,&y))
	{
		pMultiEdit->SetFont((*pScript)(iString,x).c_str(),atoi((*pScript)(iString,y).c_str()));
	}
	//	установка региона
	GetRegion(pScript,iString).GetRect(&left,&top,&right,&bottom);
	pMultiEdit->SetRegion(left,top,right,bottom);
	//	установка свойств
	if(pScript->FindInRow(g_pMultiEditCaret,0,&x))
	{
		MultiEdit::Traits tr;

		tr.m_CaretImageName = (*pScript)(iString,x).c_str();
		pMultiEdit->SetTraits(tr);
	}
	//	установка текста
	if(pScript->FindInRow(g_pMultiEditText,0,&x) && (*pScript)(iString,x).size())
	{
//		pMultiEdit->SetText((*pScript)(iString,x).c_str());
	}

	SetTip(pMultiEdit,pScript,iString);

	return pMultiEdit;
}
