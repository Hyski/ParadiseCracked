/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   04.09.2000

************************************************************************/
#include "Precomp.h"
#include "GFont.h"
#include "FormattedText.h"

//---------- Лог файл ------------
#ifdef _DEBUG_FONT
CLog ft_log;
#define ft	ft_log["ft.log"]
#else
#define ft	/##/
#endif

#define min(a,b)				(a<b?a:b)

FormattedText::FormattedText()
{
	m_uiTabSize = 4;
}

FormattedText::~FormattedText()
{
}
/*
	Комментарий:
		на этапе составления строки
	 - если tFormat == CENTER,LEFT,RIGHT, то uiAddSpace == 0 всегда;
	 - если tFormat == WIDE, то uiAddSpace рассчитывается исходя из uiRectWidth;
	 - если tFormat == CENTER,RIGHT, то uiOffset рассчитывается.
		на этапе общего представления о тексте


*/
void FormattedText::Resolve(const char *pText,
							GFont& gFont,
							TEXT_FORMAT tFormat,
							TEXT_ALIGN tAlign,
							unsigned int uiRectWidth,
							unsigned int uiRectHeight)
{
	FORMATTED_STRING fs;
	int len = strlen(pText);

	ft("\n--- Resolve Text ---\n");
	ft("    ...parametrs: tFormat[%d] tAlign[%d] rect[%d:%d]\n",tFormat,tAlign,uiRectWidth,uiRectHeight);
	ft("text: [%s]\n",pText);
	//	запоминаем параметры
	m_tFormat = tFormat;
	m_tAlign = tAlign;
	//	очищаем данные
	m_vFS.clear();
	//	сканируем текст
	for(int i=0;i<len;i++)
	{
		i += GetString(pText+i,gFont,uiRectWidth,&fs);
		ft("\ni: %d; len: %d\n",i,len);
		ft("getting string:\n");
		for(int j=0;j<fs.vWords.size();j++)
			ft(": [%s]\n",fs.vWords[j].c_str());
		//	рассчитываем смещение от края
		switch(tFormat)
		{
		case TF_RIGHT:
			fs.uiOffset = uiRectWidth - fs.uiWidth;
			break;
		case TF_CENTER:
			fs.uiOffset = (uiRectWidth - fs.uiWidth)/2;
			break;
		}
		//	обрезаем uiAddSpace
		if(tFormat!=TF_WIDE)
			fs.uiAddSpace = 0;
		//	добавляем 
		m_vFS.push_back(fs);
	}
	//	расчитываем метрики по высоте
	m_uiTopOffset = 0;
	m_uiTextHeight = m_vFS.size()*gFont.Height();
	if(!(m_uiTextHeight>uiRectHeight))
	{
		if(tAlign == T_BOTTOM)
			m_uiTopOffset = uiRectHeight-m_uiTextHeight;
		if(tAlign == T_CENTER)
			m_uiTopOffset = (uiRectHeight-m_uiTextHeight)/2;
	}
}

int FormattedText::GetString(const char *pText,GFont& gFont,unsigned int uiRectWidth,FORMATTED_STRING *pFS)
{
	int len = strlen(pText);
	std::string word;
	SIZE sz,szSpace;
	int str_width = 0;

	ft("     ...resolve string:\n");
	//	очищаем данные
	pFS->vWords.clear();
	pFS->uiAddSpace = 0;
	pFS->uiOffset = 0;
	pFS->uiWidth = 0;
	pFS->bEndOfString = false;
	//	находим размер символа пробел
	gFont.GetSize(' ',&szSpace);
	str_width = -szSpace.cx;
	//	сканируем текст
	int i;
	for(i=0;i<len;i++)
	{
		word.clear();
		//	получаем слово
		i += GetWord(pText+i,word);
		if(word.size())
		{
			//	получаем ширину слова в заданном шрифте 
			gFont.GetSize(word.c_str(),&sz);
			ft("     - word length: %d of [%s];\n",sz.cx,word.c_str());
			//	получаем общий размер строки
			str_width += sz.cx+szSpace.cx;
			//	проверяем за выход за пределы
			if(str_width > uiRectWidth)
			{//	выходит за пределы
				i -= (word.size()+1);
				str_width -= (sz.cx+szSpace.cx);
				ft("     - str_width: %d of uiRectWidth[%d];\n",str_width,uiRectWidth);
				break;
			}
			//	добавляем слово в массив
			pFS->vWords.push_back(word);
		}
		//	проверяем на конец строки
		if(pText[i] == '\n')
		{//	конец строки
			ft("     - the end of string;\n");
			pFS->bEndOfString = true;
			break;
		}
//		ft("i: %d; len: %d\n",i,len);
	}
	//	хоть одно слово должно быть в строке
	if(pFS->vWords.size()==0)
	{
		i += word.size();
		pFS->vWords.push_back(word);
	}
	//	формируем информацию о строке
	if((uiRectWidth>str_width) && (pFS->vWords.size()>1))
		pFS->uiAddSpace = (uiRectWidth-str_width)/(pFS->vWords.size()-1);
	//	запоминаем ширину строки
	pFS->uiWidth = min(str_width,uiRectWidth);

	ft("     - uiAddSpace: %d\n",pFS->uiAddSpace);
	ft("     - uiWidth: %d\n",pFS->uiWidth);

	return min(i,len);
}

int FormattedText::GetWord(const char *pText,std::string& word)
{
	int len = strlen(pText);
	bool bNoSpaceFind = false;

	int i;
	for(i=0;i<len;i++)
	{
		//	слово начинается с пробелов
		if(pText[i] == '\t')
		{
			for(int j=0;j<m_uiTabSize;j++)
				word += ' ';
			continue;
		}
		if((bNoSpaceFind && pText[i] == ' ') || (pText[i] == '\n'))
			break;
		word += pText[i];
		if(pText[i] != ' ')
			bNoSpaceFind = true;
	}
	ft("      - find: [%s]\n",word.c_str());

	return i;
}


