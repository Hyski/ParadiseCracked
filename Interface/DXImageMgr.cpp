/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   02.03.2001

************************************************************************/
#include "Precomp.h"
#include "../Common/SurfaceMgr/SurfaceMgr.h"
#include "../Common/BinkMgr/BinkMgr.h"
#include "DXImageMgr.h"

//---------- Ћог файл ------------
#ifdef _DEBUG_GRAPHICS
CLog dximgmgr_log;
#define dximgmgr	dximgmgr_log["dximgmgr.log"]
#else
#define dximgmgr	/##/
#endif
//--------------------------------

DXImageMgr::Deleter DXImageMgr::m_Deleter;

#pragma message("DXImageMgr: Ёто testing!")
int g_Counter = 0;

DXImageMgr::DXImageMgr()
{
}

DXImageMgr::~DXImageMgr()
{
}
//	создать образ из чего-то (автоматическое определение)
DXImage* DXImageMgr::CreateImage(const char *pName)
{
	static char path[MAX_PATH];
	static char drive[_MAX_DRIVE];
	static char dir[_MAX_DIR];
	static char fname[_MAX_FNAME];
	static char ext[_MAX_EXT];

	g_Counter++;

	strcpy(path,pName);
	_strlwr(path);
	_splitpath(path,drive,dir,fname,ext);
	//	считываем бинки
	if(!strcmp(ext,".bik"))
		return CreateBink(pName);

	//	во всех остальных случа€х:
	return CreatePicture(pName);
}
//	создать образ заданного размера
DXImage* DXImageMgr::CreateImage(unsigned int width,unsigned int height)
{
	static std::string name;
		
	g_Counter++;

	name = SurfaceMgr::Instance()->CreateSurface(width,height);	
	dximgmgr("create image for width and height <%s>;\n",name.c_str());

	return new DXImage(SurfaceMgr::Instance()->Surface(name.c_str()),name.c_str(),DXImage::T_PICTURE);
}
//	создать копию образа с определенным именем
DXImage* DXImageMgr::CreateImage(const char* name,const DXImage* image)
{
	g_Counter++;
	return new DXImage(SurfaceMgr::Instance()->CreateSurface(name,image->m_pSurface),name,DXImage::T_PICTURE);
}
//	получить указатель на класс по имени
DXImage* DXImageMgr::CreatePicture(const char *pName)
{
	dximgmgr("Loading picture <%s>\n",pName);
	return new DXImage(SurfaceMgr::Instance()->Surface(pName),pName,DXImage::T_PICTURE);
}

DXImage* DXImageMgr::CreateBink(const char *pName)
{
	dximgmgr("Loading bink <%s>\n",pName);
	return new DXImage(BinkMgr::Instance()->Surface(pName),pName,DXImage::T_BINK);
}
//	перезагрузить образ
void DXImageMgr::ReloadImage(DXImage* image)
{
	dximgmgr("reloading image;\n");
	switch(image->m_Type)
	{
	case DXImage::T_PICTURE:
		image->m_pSurface = SurfaceMgr::Instance()->Surface(image->m_Resource.c_str());
		break;
	case DXImage::T_BINK:
		image->m_pSurface = BinkMgr::Instance()->Surface(image->m_Resource.c_str());
		break;
	}
}
//	уничтожить все ресурсы, св€занные с данным экземпл€ром и сам экземпл€р
void DXImageMgr::Release(DXImage* image)
{
	switch(image->m_Type)
	{
	case DXImage::T_PICTURE:
		SurfaceMgr::Instance()->Release(image->m_Resource.c_str());
		break;
	case DXImage::T_BINK:
		BinkMgr::Instance()->Release(image->m_Resource.c_str());
		break;
	}
	delete image;

	g_Counter--;

	dximgmgr("counter [%0.6d];\n",g_Counter);
}
//	узнать размер образа
void DXImageMgr::GetImageSize(DXImage* image,int* width,int* height)
{
	DDSURFACEDESC2 ddsd;

	if(image && image->m_pSurface)
	{
		memset(&ddsd,0,sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		image->m_pSurface->GetSurfaceDesc(&ddsd);
		*width  = ddsd.dwWidth;
		*height = ddsd.dwHeight;
	}
}
//	узнать, загружен ли такой образ или нет
bool DXImageMgr::IsImageCreated(const char* name) const
{
	return SurfaceMgr::Instance()->IsSurfaceCreated(name);
}

//***************************************************************//
//********************** class DXImage **************************//
DXImage::DXImage(LPDIRECTDRAWSURFACE7 surface,const char* resource,TYPE type)
{
	m_pSurface = surface;
	m_Resource = resource;
	m_Type = type;
}

DXImage::~DXImage()
{
	dximgmgr("destroy image;\n");
}

void DXImage::Reload(void)
{
	m_pSurface = 0;
	DXImageMgr::Instance()->ReloadImage(this);
}

void DXImage::Release(void)
{
	m_pSurface = 0;
	DXImageMgr::Instance()->Release(this);
}
