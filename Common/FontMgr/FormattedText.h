/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   04.09.2000

************************************************************************/
#ifndef _FORMATTED_TEXT_H_
#define _FORMATTED_TEXT_H_

typedef struct 
{
	std::vector<std::string> vWords;		//	вектор слов
	unsigned int uiAddSpace;				//	дополнительное расстояние между словами
	unsigned int uiOffset;					//	смещение от начала
	unsigned int uiWidth;					//	ширина строки этим шрифтом
	bool bEndOfString;						//	конец строки (абзаца)
} FORMATTED_STRING;

class GFont;

class FormattedText
{
public:
	enum TEXT_FORMAT	{TF_LEFT			= 0x0,			
						 TF_RIGHT			= 0x1,			
						 TF_CENTER			= 0x2,			
						 TF_WIDE			= 0x3			
						};	
	enum TEXT_ALIGN		{T_TOP				= 0x0,			
						 T_BOTTOM			= 0x1,			
						 T_CENTER			= 0x2			
						};
private:
	TEXT_FORMAT m_tFormat;
	TEXT_ALIGN m_tAlign;
private:
	unsigned int m_uiTextHeight;
	unsigned int m_uiTopOffset;
public:
	std::vector<FORMATTED_STRING> m_vFS;
	unsigned int m_uiTabSize;
public:
	FormattedText();
	virtual ~FormattedText();
public:
	void Resolve(const char *pText,
				 GFont& gFont,
				 TEXT_FORMAT tFormat,
				 TEXT_ALIGN tAlign,
				 unsigned int uiRectWidth,
				 unsigned int uiRectHeight);
	unsigned int TopOffset(void);
private:
	int GetString(const char *pText,GFont& gFont,unsigned int uiRectWidth,FORMATTED_STRING *pFS);
	int GetWord(const char *pText,std::string& word);

};

inline unsigned int FormattedText::TopOffset(void)
{
	return m_uiTopOffset;
}

#endif	//_FORMATTED_TEXT_H_