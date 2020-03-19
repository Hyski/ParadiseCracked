/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   27.02.2001

************************************************************************/
#include "Precomp.h"
#include "DDSurface.h"
#include "../Image/Bitmap.h"
#include "../Image/Targa.h"
#include "../Image/Jpeg.h"
#include "SurfaceMgr.h"

//---------- Лог файл ------------
#ifdef _DEBUG_GRAPHICS
CLog srfmgr_log;
#define srfmgr	srfmgr_log["srfmgr.log"]
#else
#define srfmgr	/##/
#endif
//--------------------------------

SurfaceMgr::Deleter SurfaceMgr::m_Deleter;

SurfaceMgr::SurfaceMgr() 
{
}

SurfaceMgr::~SurfaceMgr()
{
	Release();
}

//	получить поверхность по идентификатору
LPDIRECTDRAWSURFACE7 SurfaceMgr::Surface(const char *pName)
{
	static std::map<std::string,Reference*>::iterator i;

	i = m_mSurfaces.find(pName);
	if(i == m_mSurfaces.end())
	{
		srfmgr("Loading surface [%s] - ",pName);
		LoadImageToSurface(pName);
		i = m_mSurfaces.find(pName);
	}
	srfmgr("Return surface [%s] - reference[%d];\n",pName,i->second->Counter());

	return i->second->Get()->Surface();
}
//	уничтожить поверхность по идентификатору
void SurfaceMgr::Release(const char *pName)
{
	std::map<std::string,Reference*>::iterator i;

	i = m_mSurfaces.find(pName);
	if(i!=m_mSurfaces.end())
	{
		if(!i->second->Release())
		{
			srfmgr("Unload surface: [%s];\n",pName);
			delete i->second;
			m_mSurfaces.erase(i);
		}
	}
}
//	уничтожить все поверхности
void SurfaceMgr::Release(void)
{
	srfmgr(" --- Releasing All Surfaces --- \n");
	for(std::map<std::string,Reference*>::iterator i = m_mSurfaces.begin();i!=m_mSurfaces.end();i++)
	{
		srfmgr("Unload surface: [%s];\n",i->first.c_str());
		delete i->second;
	}
	m_mSurfaces.clear();
}

//	создать поверхность заданного размера и получить ее идентификатор
const char* SurfaceMgr::CreateSurface(unsigned int width,unsigned int height)
{
	Image image;
	DDSurface *surface;
	static char name[100];
	static int counter = 0;

	srfmgr("Creating surface: size[%dx%d];\n",width,height);
	//	генерируем имя поверхности
	sprintf(name,"dynamic%0.8x%0.8x%0.8ld",width,height,counter++);
	srfmgr("    ...name: [%s];\n",name);
	image.Create(width,height,&Image::ic_PixelFormat(32,Image::ic_PixelFormat::T_ARGB888));
	surface = new DDSurface(D3DKernel::GetDD(),&image);
//	surface->Create(D3DKernel::GetDD(),&image);
	m_mSurfaces.insert(std::map<std::string,Reference*>::value_type(name,new Reference(surface)));
	SetColorKey(surface);

	return name;
}
//	если данный идентификатор уже используется то return 0
LPDIRECTDRAWSURFACE7 SurfaceMgr::CreateSurface(const char* name,LPDIRECTDRAWSURFACE7 surface)
{
	static std::map<std::string,Reference*>::iterator i;

	i = m_mSurfaces.find(name);
	if(i == m_mSurfaces.end())
	{
		DDSurface* dds = new DDSurface(D3DKernel::GetDD(),surface);
		m_mSurfaces.insert(std::map<std::string,Reference*>::value_type(name,new Reference(dds)));
		SetColorKey(dds);
		return m_mSurfaces[name]->Get()->Surface();
	}

	return m_mSurfaces[name]->Get()->Surface();
}
//	узнать, создана ли поверхность с таким идентификатором
bool SurfaceMgr::IsSurfaceCreated(const char* name) const
{
	return (m_mSurfaces.find(name)!=m_mSurfaces.end())?true:false;
}

//*******************************************************************************//
void SurfaceMgr::LoadImageToSurface(const char *pName)
{
	DDSurface *pSurface = new DDSurface;
	VFile* image = DataMgr::Load(pName);

	if(image->Size())
	{
		switch(GetImageType(image->Extension()))
		{
		case IT_TARGA:
			{
				Targa* tga = new Targa();
				tga->Load(image->Data(),image->Size());
				pSurface->Create(D3DKernel::GetDD(),tga);
				delete tga;
			}
			break;
		case IT_JPEG:
			{
				Jpeg* jpeg = new Jpeg();
				jpeg->Load(image->Data(),image->Size());
				pSurface->Create(D3DKernel::GetDD(),jpeg);
				delete jpeg;
			}
			break;
		case IT_BITMAP:
			{
				Bitmap* bmp = new Bitmap();
				bmp->Load(image->Data(),image->Size());
				pSurface->Create(D3DKernel::GetDD(),bmp);
				delete bmp;
			}
			break;
		}
	}
	SetColorKey(pSurface);
	m_mSurfaces.insert(std::map<std::string,Reference*>::value_type(pName,new Reference(pSurface)));
	DataMgr::Release(pName);
}

void SurfaceMgr::SetColorKey(DDSurface *surface)
{
	DDPIXELFORMAT ddpf;
	DDCOLORKEY ddcolorkey;

	if(surface->Surface())
	{
		memset(&ddpf,0,sizeof(DDPIXELFORMAT));
		ddpf.dwSize = sizeof(DDPIXELFORMAT);
		D3DKernel::GetPS()->GetPixelFormat(&ddpf);
		ddcolorkey.dwColorSpaceLowValue = ddpf.dwGBitMask|ddpf.dwBBitMask;
		ddcolorkey.dwColorSpaceHighValue = ddcolorkey.dwColorSpaceLowValue;
		surface->Surface()->SetColorKey(DDCKEY_SRCBLT,&ddcolorkey);
	}
}

SurfaceMgr::IMAGE_TYPE SurfaceMgr::GetImageType(const char* pFileExtension)
{
	if(!stricmp(pFileExtension,".tga")) return IT_TARGA;
	if(!stricmp(pFileExtension,".jpg")) return IT_JPEG;
	if(!stricmp(pFileExtension,".bmp")) return IT_BITMAP;

	return IT_UNKNOWN;
}

//	вернуть размер памяти, занимаемый поверхностями
int SurfaceMgr::Memory(void) 
{
	LPDIRECTDRAWSURFACE7 surface;
	DDSURFACEDESC2 ddsd;
	int counter = 0;

	for(std::map<std::string,Reference*>::iterator i=m_mSurfaces.begin();i!=m_mSurfaces.end();i++)
	{
		surface = i->second->Get()->Surface();
		i->second->Release();

		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		if(surface)
		{
			surface->GetSurfaceDesc(&ddsd);
			counter += ddsd.dwHeight*ddsd.lPitch;
		}
	}

	return counter;
}

//***************************************************************//
//************** class SurfaceMgr::Reference ********************//
SurfaceMgr::Reference::Reference() : m_Value(0),m_Counter(0)
{
}

SurfaceMgr::Reference::Reference(DDSurface* value)
{
	m_Value = value;
	m_Counter = 0;
}

SurfaceMgr::Reference::~Reference()
{
	if(m_Value)
		delete m_Value;
}
