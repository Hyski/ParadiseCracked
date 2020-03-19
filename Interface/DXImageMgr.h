/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#ifndef _DX_IMAGE_MGR_H_
#define _DX_IMAGE_MGR_H_

#include "../Common/UI/Widget.h"

class DXImage;

class DXImageMgr
{
private:
	struct Deleter 
	{
	public:
		DXImageMgr *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	DXImageMgr();
	virtual ~DXImageMgr();
public:
	//	создать образ из чего-то (автоматическое определение)
	DXImage* CreateImage(const char *pName);
	//	создать образ заданного размера
	DXImage* CreateImage(unsigned int width,unsigned int height);
	//	создать копию образа с определенным именем
	DXImage* CreateImage(const char* name,const DXImage* image);
	//	узнать размер образа
	void GetImageSize(DXImage* image,int* width,int* height);
	//	узнать, загружен ли такой образ или нет
	bool IsImageCreated(const char* name) const;
	//	перезагрузить образ
	void ReloadImage(DXImage* image);
	//	уничтожить образ 
	void Release(DXImage* image);
private:
	//	создать образ из картинки (tga,bmp,jpg)
	DXImage* CreatePicture(const char *pName);
	//	создать образ из бинка (bik)
	DXImage* CreateBink(const char *pName);
public:
	static DXImageMgr *Instance(void);
};

inline DXImageMgr* DXImageMgr::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new DXImageMgr();
	return m_Deleter.m_pInstance;
}

//***************************************************************//
//********************** class DXImage **************************//
class DXImage : public WidgetImage
{
public:
	enum TYPE
	{
		T_UNKNOWN,
		T_PICTURE,
		T_BINK
	};
public:
	std::string m_Resource;		//	имя ресурса (катинки, бика)
	TYPE m_Type;
	LPDIRECTDRAWSURFACE7 m_pSurface;
public:
	DXImage(LPDIRECTDRAWSURFACE7 surface,const char* resource,TYPE type);
	virtual ~DXImage();
public:
	void Reload(void);
	void Release(void);
};

#endif	//_DX_IMAGE_MGR_H_