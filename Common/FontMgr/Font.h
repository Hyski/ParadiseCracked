/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   06.03.2001

************************************************************************/
#ifndef _FONT_H_
#define _FONT_H_

class Font
{
protected:
	int m_iFontHeight;
	RECT m_rcChar[256];
	ABC m_abcChar[256];
public:
	Font();
	virtual ~Font();
public:
	int Height(void) const;
	void GetSize(unsigned char ch,int *width,int *height) const;
	void GetSize(const char *pText,int *width,int *height) const;
};

inline int Font::Height(void) const
{
	return m_iFontHeight;
}

inline void Font::GetSize(unsigned char ch,int *width,int *height) const
{
	(*width)  = m_abcChar[ch].abcA +
			    m_abcChar[ch].abcC +
			    m_abcChar[ch].abcB;
	(*height) = m_iFontHeight;
}

inline void Font::GetSize(const char *pText,int *width,int *height) const
{
	int len = strlen(pText);

	(*width) = 0;
	(*height) = 0;
	for(int i=0;i<len;i++)
	{
		(*width) += m_abcChar[(unsigned char)pText[i]].abcA +
				    m_abcChar[(unsigned char)pText[i]].abcC +
				    m_abcChar[(unsigned char)pText[i]].abcB;
	}
	(*height) = m_iFontHeight;
}

#endif	//_FONT_H_