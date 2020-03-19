/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   09.09.2000

************************************************************************/
#include "Precomp.h"
#include "mlff.h"
#include "../TextureMgr/DIBData.h"
#include "../../globals.h"
#include "FastFont.h"
#include "../graphpipe/vshader.h"
//---------- Лог файл ------------
#ifdef _NODEBUG_FONT
CLog ffont_log;
#define ffont	ffont_log["ffont.log"]
#else
#define ffont	/##/
#endif

FastFont::FastFont()
{
	m_uiChars = 0;
	m_iTexWidth = 0;
	m_iTexHeight = 0;
}

FastFont::~FastFont()
{
}

bool FastFont::Load(LPDIRECT3DDEVICE7 lpd3dDevice,VFile *pVFile)
{
	MLFF_FILEHEADER fh;
	MLFF_DATAHEADER dh;

	if(pVFile->Size()!=0)
	{
		//	на начало файла
		pVFile->Rewind();
		//	считываем заголовок файла
		if(pVFile->Read(&fh,sizeof(MLFF_FILEHEADER)))
		{
			//	перемещаем указатель на начало заголовка данных
			pVFile->Seek(fh.lSize,VFile::VFILE_SET);
			//	считываем заголовок данных
			if(pVFile->Read(&dh,sizeof(MLFF_DATAHEADER)))
			{
				//	перемещаем указатель на начало битового образа
				pVFile->Seek(dh.lSize+fh.lSize,VFile::VFILE_SET);
				//	заполняем общие данные
				m_iFontHeight = dh.lFontHeight;
				memcpy(m_rcChar,dh.rcChar,sizeof(m_rcChar));
				memcpy(m_abcChar,dh.abcChar,sizeof(m_abcChar));
				//	проверяем правильность файла
				if((pVFile->Size()-pVFile->Tell()) == dh.lWidth,dh.lHeight)
				{
					//	создаем поверхность и копируем туда битмапку
					return CreateTexture(lpd3dDevice,dh.lWidth,dh.lHeight,pVFile);
				}
			}
		}
	}

	return false;
}

bool FastFont::CreateTexture(LPDIRECT3DDEVICE7 lpd3dDevice,int width,int height,VFile *pVFile)
{
	DWORD *pImage;

	if(m_SrcImage.Create(width,height,32))
	{
		pImage = (DWORD *)m_SrcImage.Image();
		for(int i=(height-1);i>=0;i--)		
		{
			for(int j=0;j<width;j++)		
			{
				pImage[(i*width)+j] = (pVFile->ReadChar()<<24)+0xffffff;
			}
		}
		//	создаем текстуру
		if(m_d3dTexture.Create(lpd3dDevice,&m_SrcImage,0,0))
		{
			m_iTexWidth = m_d3dTexture.Width();
			m_iTexHeight = m_d3dTexture.Height();

			return true;
		}
	}

	return false;
}

void FastFont::Tune(LPDIRECT3DDEVICE7 lpd3dDevice)
{
	//	создаем текстуру
	if(m_d3dTexture.Create(lpd3dDevice,&m_SrcImage,0,0))
	{
		m_iTexWidth = m_d3dTexture.Width();
		m_iTexHeight = m_d3dTexture.Height();
	}
}

//	вывод текста (виртуальные координаты)
void FastFont::RenderText(const char *pText,int vX,int vY,DWORD dwColor)
{
	int len = strlen(pText);
	char ch;
	float left,top,width,height;

	ffont("--- RenderText: [%s] ---\n",pText);
	for(int i=0;i<len;i++)
	{
		ch = pText[i]-32;
		ffont("char: %c\n",ch+32);

		vX += m_abcChar[ch].abcA;

		left	= ToScreenX(vX);
		top		= ToScreenY(vY);
		width	= ToScreenX(m_rcChar[ch].right);
		height	= ToScreenY(m_rcChar[ch].bottom);

		m_d3dTLVertex[m_uiChars][0] = D3DTLVERTEX(D3DVECTOR(left,top,0),1,dwColor,0.0f,(float)m_rcChar[ch].left/m_iTexWidth,(float)m_rcChar[ch].top/m_iTexHeight);
		m_d3dTLVertex[m_uiChars][1] = D3DTLVERTEX(D3DVECTOR(left+width,top,0),1,dwColor,0.0f,(float)(m_rcChar[ch].left+m_rcChar[ch].right)/m_iTexWidth,(float)m_rcChar[ch].top/m_iTexHeight);
		m_d3dTLVertex[m_uiChars][2] = D3DTLVERTEX(D3DVECTOR(left+width,top+height,0),1,dwColor,0.0f,(float)(m_rcChar[ch].left+m_rcChar[ch].right)/m_iTexWidth,(float)(m_rcChar[ch].top+m_rcChar[ch].bottom)/m_iTexHeight);
		m_d3dTLVertex[m_uiChars][3] = D3DTLVERTEX(D3DVECTOR(left,top,0),1,dwColor,0.0f,(float)m_rcChar[ch].left/m_iTexWidth,(float)m_rcChar[ch].top/m_iTexHeight);
		m_d3dTLVertex[m_uiChars][4] = D3DTLVERTEX(D3DVECTOR(left,top+height,0),1,dwColor,0.0f,(float)m_rcChar[ch].left/m_iTexWidth,(float)(m_rcChar[ch].top+m_rcChar[ch].bottom)/m_iTexHeight);
		m_d3dTLVertex[m_uiChars][1] = D3DTLVERTEX(D3DVECTOR(left+width,top,0),1,dwColor,0.0f,(float)(m_rcChar[ch].left+m_rcChar[ch].right)/m_iTexWidth,(float)m_rcChar[ch].top/m_iTexHeight);
		m_d3dTLVertex[m_uiChars][5] = D3DTLVERTEX(D3DVECTOR(left+width,top+height,0),1,dwColor,0.0f,(float)(m_rcChar[ch].left+m_rcChar[ch].right)/m_iTexWidth,(float)(m_rcChar[ch].top+m_rcChar[ch].bottom)/m_iTexHeight);
		vX += m_abcChar[ch].abcC+m_abcChar[ch].abcB;
		m_uiChars++;
		if(m_uiChars == MAX_CHAR_NUM)
			SwapBuffer();
	}
}

void FastFont::SwapBuffer(void)
{
	D3DKernel::GetD3DDevice()->SetTexture(0,m_d3dTexture.Surface());
	
	StatesManager::SetRenderState(D3DRENDERSTATE_CULLMODE,D3DCULL_NONE);
	StatesManager::SetRenderState(D3DRENDERSTATE_FILLMODE,D3DFILL_SOLID);
	
	StatesManager::SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE);
	
	StatesManager::SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA);
	StatesManager::SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA);
	
	StatesManager::SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE,FALSE);
	
	StatesManager::SetRenderState(D3DRENDERSTATE_ZBIAS,0);
	StatesManager::SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,TRUE);
	StatesManager::SetRenderState(D3DRENDERSTATE_ZFUNC,D3DCMP_ALWAYS);
	
	StatesManager::SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_CLAMP);
	StatesManager::SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
	StatesManager::SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
	StatesManager::SetTextureStageState(0,D3DTSS_ADDRESS,D3DTADDRESS_CLAMP);
	StatesManager::SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
	StatesManager::SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
	//	-----------------------------------------------------------------------------------------
	D3DKernel::GetD3DDevice()->DrawPrimitive(D3DPT_TRIANGLELIST,D3DFVF_TLVERTEX,
		m_d3dTLVertex,m_uiChars*6,D3DDP_WAIT);

	m_uiChars = 0;
}
