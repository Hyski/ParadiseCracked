/***********************************************************************

                             Texture Manager

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   17.05.2001

************************************************************************/
#ifndef _TEXTURE_FORMAT_MGR_H_
#define _TEXTURE_FORMAT_MGR_H_

enum COMPRESSED_FORMAT
{
	CF_NONE,
	CF_DXT1,	// хорошо для ARGB:1555 (нет альфы или 1 бит на альфу)
	CF_DXT2,	// хорошо для ARGB:4444 (явная альфа premultiplied) 
	CF_DXT3,	// хорошо для ARGB:4444 (явная альфа non premultiplied)
	CF_DXT4,	// хорошо для ARGB:8888 (интерполированная альфа premultiplied)
	CF_DXT5		// хорошо для ARGB:8888 (интерполированная альфа non premultiplied) 
};

enum PALETTE_FORMAT
{
	PF_NONE = 0,
	PF_1BIT = 1,
	PF_2BIT = 2,
	PF_4BIT = 4,
	PF_8BIT = 8
};

struct PixelFormat
{
	DWORD m_AlphaBitCount;		//	кол-во бит на альфу
	DWORD m_RedBitCount;		//	кол-во бит на красный
	DWORD m_GreenBitCount;		//	кол-во бит на зеленый
	DWORD m_BlueBitCount;		//	кол-во бит на синий
	DDPIXELFORMAT m_ddpf;		//	собственно пиксел формат
	COMPRESSED_FORMAT m_cf;		//	тип компрессии, если присутсвует
	PALETTE_FORMAT m_pf;		//	формат палитры
	PixelFormat();
	PixelFormat(LPDDPIXELFORMAT pDDPF);
};

class TextureFormatMgr
{
private:
	struct Deleter 
	{
	public:
		TextureFormatMgr *m_pInstance;
	public:
		Deleter():m_pInstance(0){}
		~Deleter(){if(m_pInstance) delete m_pInstance;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	std::vector<PixelFormat> m_vPFs;
private:
	TextureFormatMgr();
	virtual ~TextureFormatMgr();
public:
	void Init(LPDIRECT3DDEVICE7 lpd3dDevice);
	void Release(void);
	//	получить близкий формат текстур к заданному
	bool ChoosePixelFormat(DDPIXELFORMAT* pDDPF);
public:
	static const char* GetFourCCName(DWORD dwFourCC);
	static COMPRESSED_FORMAT GetFourCCValue(DWORD dwFourCC);
	static PALETTE_FORMAT GetPaletteIndexed(DWORD dwFlags);
public:
	static TextureFormatMgr *Instance(void);
};

inline TextureFormatMgr* TextureFormatMgr::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new TextureFormatMgr();
	return m_Deleter.m_pInstance;
}

#endif	//_TEXTURE_FORMAT_MGR_H_