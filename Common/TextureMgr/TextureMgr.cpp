/***********************************************************************

                             Texture Manager

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   16.05.2001

************************************************************************/
#include "Precomp.h"
#include "TextureFormatMgr.h"
#include "D3DTexture.h"
#include "DIBData.h"
#include "../Image/Bitmap.h"
#include "../Image/Targa.h"
#include "../Image/Jpeg.h"
#include "TextureMgr.h"

//---------- Лог файл ------------
#ifdef _HOME_VERSION
CLog txrmgr_log;
#define txrmgr	txrmgr_log["txrmgr.log"]
#else
#define txrmgr	/##/
#endif
//--------------------------------

TextureMgr::Deleter TextureMgr::m_Deleter;
TextureMgr::TextureMgr() : m_DefaultMipMapLevel(0),m_DefaultQuality(0),m_lpd3dDevice(0)
{
	m_UseCompressing = false;
	m_UsePremultipliedAlpha = false;
}

TextureMgr::~TextureMgr()
{
}

void TextureMgr::Init(LPDIRECT3DDEVICE7 lpd3dDevice,	
					  int iDefaultMipMapLevel,			
					  unsigned int iDefaultQuality,		
					  bool bCompressed,					
					  bool bAlphaPremultiplied)
{
	//	iDefaultMipMapLevel - [-1,0,1,2,3, ...]. [-1] - все возможные mipmap'ы, [0] - нет мипмэпов
	//	iDefaultQuality - [0,1,2,3,...] 0 - лучшее качество
	txrmgr("    default mipmap level:      %d\n",iDefaultMipMapLevel);
	txrmgr("    default quality:           %d\n",iDefaultQuality);
	txrmgr("    using comressing:          %s\n",bCompressed?"yes":"no");
	txrmgr("    using premultiplied alpha: %s\n",bAlphaPremultiplied?"yes":"no");
	m_lpd3dDevice			= lpd3dDevice;
	m_DefaultMipMapLevel	= iDefaultMipMapLevel;
	m_DefaultQuality		= iDefaultQuality;
	m_UseCompressing		= bCompressed;
	m_UsePremultipliedAlpha = bAlphaPremultiplied;
	//	инициализация менеджера поддерживаемых форматов текстур
	TextureFormatMgr::Instance()->Init(m_lpd3dDevice);

	txrmgr("TextureMgr::Init success;\n");
}

LPDIRECTDRAWSURFACE7 TextureMgr::Texture(const char *pTextureName)
{
	return Texture(pTextureName,m_DefaultMipMapLevel,m_DefaultQuality);
}

LPDIRECTDRAWSURFACE7 TextureMgr::Texture(const char *pTextureName,
										 int iMipMapLevel,
										 unsigned int iQuality)
{
	static std::map<std::string,Reference*>::iterator i;
	D3DTexture* texture;

	i = m_Textures.find(pTextureName);
	if(i == m_Textures.end())
	{
		txrmgr("Loading texture [%s] - ",pTextureName);
		texture = new D3DTexture();
		if(LoadImageToSurface(texture,pTextureName,iMipMapLevel,iQuality))
		{
			m_Textures.insert(std::map<std::string,Reference*>::value_type(pTextureName,new Reference(texture)));
			i = m_Textures.find(pTextureName);
		}
		else
		{
			delete texture;
			return 0;
		}
	}
//	txrmgr("Referenced texture [%s] - [%d]\n",pTextureName,i->second->Counter());

	return i->second->Get()->Surface();
}

LPDIRECTDRAWSURFACE7 TextureMgr::CreateTexture(const char *pTextureName,
											   DIBData *pImage,
											   int iMipMapLevel,
											   unsigned int iQuality)
{
	std::map<std::string,Reference*>::iterator i;
	D3DTexture* texture;

	i = m_Textures.find(pTextureName);
	if(i!=m_Textures.end())
		throw CASUS(std::string(std::string("Невозможно создать текстуру из битового образа. Имя <")+std::string(pTextureName)+std::string("> уже используется.\n")));
	//	создаем текстуру из битового образа
	txrmgr("Creating texture [%s];\n",pTextureName);
	texture = new D3DTexture(m_lpd3dDevice,pImage,iMipMapLevel,iQuality,false,false);
	if(texture->Surface())
	{
		m_Textures.insert(std::map<std::string,Reference*>::value_type(pTextureName,new Reference(texture)));
		return m_Textures[pTextureName]->Get()->Surface();
	}
	else
	{
		delete texture;
		return 0;
	}
}

int TextureMgr::Release(const char* pTextureName)
{
	std::map<std::string,Reference*>::iterator i;
	int count = 0;

	i = m_Textures.find(pTextureName);
	if(i!=m_Textures.end())
	{
		if(!(count = i->second->Release()))
		{
			txrmgr("Unload texture: [%s];\n",pTextureName);
			delete i->second;
			m_Textures.erase(i);
		}
	}

	return count;
}

void TextureMgr::Release(void)
{
	txrmgr("\nReleasing lost textures:\n");
	for(std::map<std::string,Reference*>::iterator i = m_Textures.begin();i!=m_Textures.end();i++)
	{
		txrmgr("Unload texture: [%s];\n",i->first.c_str());
		delete i->second;
	}
	m_Textures.clear();
}

bool TextureMgr::LoadImageToSurface(D3DTexture* texture,const char* pImageName,int iMipMapLevel,unsigned int iQuality)
{
	VFile* image = DataMgr::Load(pImageName);

	if(image->Size())
	{
		switch(GetImageType(image->Extension()))
		{
		case IT_TARGA:
			{
				Targa* targa = new Targa();
				targa->Load(image->Data(),image->Size());
				texture->Create(m_lpd3dDevice,targa,iMipMapLevel,iQuality,m_UseCompressing,m_UsePremultipliedAlpha);
				delete targa;
			}
			break;
		case IT_JPEG:
			{
				Jpeg* jpeg = new Jpeg();
				jpeg->Load(image->Data(),image->Size());
				texture->Create(m_lpd3dDevice,jpeg,iMipMapLevel,iQuality,m_UseCompressing,m_UsePremultipliedAlpha);
				delete jpeg;
			}
			break;
		case IT_BITMAP:
			{
				Bitmap* bmp = new Bitmap();
				bmp->Load(image->Data(),image->Size());
				texture->Create(m_lpd3dDevice,bmp,iMipMapLevel,iQuality,m_UseCompressing,m_UsePremultipliedAlpha);
				delete bmp;
			}
			break;
		}
	}

	DataMgr::Release(pImageName);

	txrmgr("%s",texture->Surface()?"SUCCESS\n":"FAILED\n");

	return static_cast<bool>(texture->Surface());
}
/*
TextureMgr::Reference* TextureMgr::LoadImageToSurface(const char* pImageName,int iMipMapLevel,unsigned int iQuality)
{
	D3DTexture* texture = new D3DTexture();
	VFile* image = DataMgr::Load(pImageName);

	if(image->Size())
	{
		switch(GetImageType(image->Extension()))
		{
		case IT_TARGA:
			{
				Targa* targa = new Targa();
				targa->Load(image->Data(),image->Size());
				texture->Create(m_lpd3dDevice,targa,iMipMapLevel,iQuality,m_UseCompressing,m_UsePremultipliedAlpha);
				delete targa;
			}
			break;
		case IT_JPEG:
			{
				Jpeg* jpeg = new Jpeg();
				jpeg->Load(image->Data(),image->Size());
				texture->Create(m_lpd3dDevice,jpeg,iMipMapLevel,iQuality,m_UseCompressing,m_UsePremultipliedAlpha);
				delete jpeg;
			}
			break;
		case IT_BITMAP:
			{
				Bitmap* bmp = new Bitmap();
				bmp->Load(image->Data(),image->Size());
				texture->Create(m_lpd3dDevice,bmp,iMipMapLevel,iQuality,m_UseCompressing,m_UsePremultipliedAlpha);
				delete bmp;
			}
			break;
		}
	}

	DataMgr::Release(pImageName);

	txrmgr("%s",texture->Surface()?"SUCCESS":"FAILED");

	return new Reference(texture);
}
*/
TextureMgr::IMAGE_TYPE TextureMgr::GetImageType(const char* pFileExtension)
{
	if(!stricmp(pFileExtension,".tga")) return IT_TARGA;
	if(!stricmp(pFileExtension,".jpg")) return IT_JPEG;
	if(!stricmp(pFileExtension,".bmp")) return IT_BITMAP;

	return IT_UNKNOWN;
}

int TextureMgr::Bads(void) 
{
	int counter = 0;

	for(std::map<std::string,Reference*>::iterator i = m_Textures.begin();i!=m_Textures.end();i++)
	{
		if(i->second->IsBad()) counter++;
	}

	return counter;
}
//	кол-во занимаемой памяти
int TextureMgr::Memory(void)
{
	LPDIRECTDRAWSURFACE7 surface;
	DDSURFACEDESC2 ddsd;
	int counter = 0;

	for(std::map<std::string,Reference*>::iterator i = m_Textures.begin();i!=m_Textures.end();i++)
	{
		surface = i->second->Get()->Surface();
		i->second->Release();
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		if(surface)
		{
			surface->GetSurfaceDesc(&ddsd);
			if(ddsd.ddpfPixelFormat.dwFourCC)
			{
				if(!ddsd.dwMipMapCount) counter += ddsd.dwLinearSize;
				for(;ddsd.dwMipMapCount>0;ddsd.dwMipMapCount--)
				{
					counter += ddsd.dwLinearSize;
					ddsd.dwLinearSize >>= 2;
				}
			}
			else
			{
				int size = ddsd.dwHeight*ddsd.lPitch;
				if(!ddsd.dwMipMapCount) counter += size;
				for(;ddsd.dwMipMapCount>0;ddsd.dwMipMapCount--)
				{
					counter += size;
					size >>= 2;
				}
			}
		}
	}

	return counter;
}

void TextureMgr::Report(std::vector<Description>& info)
{
	LPDIRECTDRAWSURFACE7 surface;
	DDSURFACEDESC2 ddsd;
	Description d;

	info.clear();
	for(std::map<std::string,Reference*>::iterator i = m_Textures.begin();i!=m_Textures.end();i++)
	{
		surface = i->second->Get()->Surface();
		i->second->Release();
		d.m_Name = i->first;
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		if(surface)
		{
			surface->GetSurfaceDesc(&ddsd);
			d.m_Width = ddsd.dwWidth;
			d.m_Height = ddsd.dwHeight;
			d.m_Bpp = ddsd.ddpfPixelFormat.dwRGBBitCount;
			d.m_MipMapCount = ddsd.dwMipMapCount;
			d.m_FourCC = ddsd.ddpfPixelFormat.dwFourCC;
		}
		else
		{
			d.m_Width = d.m_Height = d.m_Bpp = 0;
			d.m_MipMapCount = d.m_FourCC = 0;
		}
		info.push_back(d);
	}
}
//	кол-во загруженных текстур
int TextureMgr::Quantity(void) const
{
	return m_Textures.size();
}
//***************************************************************//
// class TextureMgr::Reference
TextureMgr::Reference::Reference(D3DTexture* value)
{
	m_Value = value;
	m_Counter = 0;
}

TextureMgr::Reference::~Reference()
{
	if(m_Value) delete m_Value;
}

bool TextureMgr::Reference::IsBad(void)
{
	if(m_Value) return !m_Value->Surface();

	return true;
}
