/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   06.03.2001

************************************************************************/
#include "Precomp.h"
#include "Font.h"
#include "LogicalText.h"

//---------- Лог файл ------------
#ifdef _NOT_LOG
CLog lt_log;
#define lt	lt_log["lt.log"]
#else
#define lt	/##/
#endif
//--------------------------------

LogicalText::LogicalText() : m_TextWidth(0),m_TextHeight(0)
{
}

LogicalText::~LogicalText()
{
}

/******************************************************************************
	На этапе составления строки
	 - если tAlign == CENTER,LEFT,RIGHT, то AddSpace == 0 всегда;
	 - если tAlign == WIDE, то AddSpace рассчитывается исходя из width_limit;
	 - если tAlign == CENTER,RIGHT, то Offset рассчитывается.
*******************************************************************************/
void LogicalText::Resolve(const char *pText,
						  const Font* pFont,
						  unsigned int tAlign,
						  unsigned int width_limit,
						  unsigned int height_limit)
{
	LogicalString ls;
	int len = strlen(pText);

	lt(" --- RESOLVE TEXT ---\n");
	m_TextWidth = 0;
	if(pText && pFont)
	{
		//	запоминаем текст
		m_SourceText = pText;
		lt("    ...text: [%s]\n\n$\n\n",pText);
		//	очищаем данные
		m_Text.clear();
		//	сканируем текст
		for(int i=0;i<len;i++)
		{
			i += GetString(pText+i,pFont,width_limit,&ls);			
			//	обрезаем m_AddSpace
			if(!(tAlign&T_WIDE)) ls.m_AddSpace = 0;
			//	рассчитываем смещение от края
			if(tAlign&T_RIGHT)
				ls.m_Offset = width_limit - ls.m_Width;
			if(tAlign&T_HCENTER)
				ls.m_Offset = (width_limit - ls.m_Width)/2;
			if(ls.m_EOS)
			{
				//	подправляем последнюю строчку
				if(tAlign&T_WIDE) ls.m_AddSpace = 0;
			}
			//	добавляем 
			m_Text.push_back(ls);
			m_TextWidth = std::max(m_TextWidth,static_cast<int>(ls.m_Width));
		}
		//	подправляем последнюю строчку
		if(tAlign&T_WIDE && m_Text.size())
			m_Text[m_Text.size()-1].m_AddSpace = 0;
		//	расчитываем метрики по высоте
		m_TopOffset = 0;
		m_TextHeight = pFont->Height()*m_Text.size();
		if(!(m_TextHeight>height_limit))
		{
			if(tAlign & T_BOTTOM)
				m_TopOffset = height_limit-m_TextHeight;
			if(tAlign & T_VCENTER)
				m_TopOffset = (height_limit-m_TextHeight)/2;
		}
		//	подсчитываем кол-во символов в форматированном тексте
		m_SymbolCount = 0;
		for(int i=(m_Text.size()-1);i>=0;i--)
		{
			m_Text[i].m_Symbols = 0;
			for(int j=(m_Text[i].m_vWords.size()-1);j>=0;j--)
			{
				m_SymbolCount += m_Text[i].m_vWords[j].size()+1;
				m_Text[i].m_Symbols += m_Text[i].m_vWords[j].size()+1;
			}
			m_SymbolCount--;
			m_Text[i].m_Symbols--;
		}
	}
}

int LogicalText::GetString(const char *pText,const Font* pFont,unsigned int width_limit,LogicalString *pLS)
{
	int len = strlen(pText);
	std::string word;
	int space_width,space_height;
	int cx,cy;
	int string_width = 0;

	lt("     ...resolve string:\n");
	//	очищаем данные
	pLS->m_vWords.clear();
	pLS->m_AddSpace = 0;
	pLS->m_Offset = 0;
	pLS->m_Width = 0;
	pLS->m_EOS = false;
	//	находим размер символа пробел
	pFont->GetSize(' ',&space_width,&space_height);
	string_width = -space_width;
	//	сканируем текст
	int i;
	for(i=0;i<len;i++)
	{
		//	получаем слово
		i += GetWord(pText+i,word);
		if(word.size())
		{
			//	получаем ширину слова в заданном шрифте 
			pFont->GetSize(word.c_str(),&cx,&cy);
			lt("     - word length: %d of [%s];\n",cx,word.c_str());
			//	получаем общий размер строки
			string_width += cx+space_width;
			//	проверяем за выход за пределы
			if(string_width > width_limit)
			{//	выходит за пределы
				i -= (word.size()+1);
				string_width -= (cx+space_width);
				break;
			}
			//	добавляем слово в массив
			pLS->m_vWords.push_back(word);
		}
		//	проверяем на конец строки
		if(pText[i] == '\n')
		{//	конец строки
			lt("     - the end of string;\n");
			pLS->m_EOS = true;
			break;
		}
	}
	//	хоть одно слово должно быть в строке
	if(!pLS->m_vWords.size())
	{
		i += word.size();
		pLS->m_vWords.push_back(word);
	}
	//	формируем информацию о строке
	if((width_limit>string_width) && (pLS->m_vWords.size()>1))
		pLS->m_AddSpace = (width_limit-string_width)/(pLS->m_vWords.size()-1);
	//	запоминаем ширину строки
	pLS->m_Width = std::min(string_width,(int)width_limit);

	return i;
}

int LogicalText::GetWord(const char *pText,std::string& word)
{
	int len = strlen(pText);
	bool bSymbolFind = false;

	word.clear();
	int i;
	for(i=0;i<len;i++)
	{
		if(pText[i] == '\n') break;
		if(pText[i] == ' ' || pText[i] == '\t')
		{
			if(bSymbolFind)	break;
			word += ' ';
		}
		else
		{
			word += pText[i];
			bSymbolFind = true;
		}
	}
	lt("      - find: [%s]\n",word.c_str());

	return i;
}

void LogicalText::Clear(void)
{
	m_Text.clear();
}
