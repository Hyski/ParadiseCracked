/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description: 
		1. Необходимо перед всеми инклудами определить D3D_OVERLOADS;
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   21.08.2000

************************************************************************/
//#define D3D_OVERLOADS
#include "Precomp.h"
#include "../../Common/TextureMgr/DIBData.h"
#include "../../Common/TextureMgr/D3DTexture.h"
#include "../../Common/TextureMgr/TextureMgr.h"
#include "../../globals.h"
#include "../Lagoon.h"
#include "../../Common/FontMgr/FontMgr.h"
#include "elText.h"
#include "../../common/graphpipe/vshader.h"

elText::elText(const char *pName) : Lagoon(pName)
{
	m_sClass = "text";
	//	-----------------------------------------
	m_uiHAlign = THA_CENTER;
	m_uiVAlign = TVA_CENTER;
	//	-----------------------------------------
	m_dwTextColor = 0xffffff;
	m_iOffsetY = 0;
	//	-----------------------------------------
	m_sText = "No Text";
	m_bTextResolved = false;
}

elText::~elText()
{
}

void elText::SetRegion(HRGN /*hRegion*/)
{
	//	такого не будет
}

void elText::Tune(void)
{
	Lagoon::Tune();
	//	---------------------------------------------
	CreateTexture();
	//	---------------------------------------------
	if(!m_bTextResolved)
		ResolveText();
	//	---------------------------------------------
	TextBlit();
}

void elText::ResolveText(void)
{
	//	форматируем текст
	m_fText.Resolve(m_sText.c_str(),*FontMgr::Instance()->GetGFont(m_sFont.c_str()),
		(m_uiHAlign==THA_LEFT)?FormattedText::TF_LEFT:(m_uiHAlign==THA_RIGHT)?FormattedText::TF_RIGHT:(m_uiHAlign==THA_CENTER)?FormattedText::TF_CENTER:FormattedText::TF_WIDE,
		(m_uiVAlign==TVA_TOP)?FormattedText::T_TOP:(m_uiVAlign==TVA_BOTTOM)?FormattedText::T_BOTTOM:FormattedText::T_CENTER,
		m_width,m_height);
	m_bTextResolved = true;
}

void elText::CreateTexture(void)
{
	DIBData image;

//	TEsT
/*	m_vTexture.m_dwVertexColor = m_dwTextColor;
	m_vTexture.Create(m_width,m_height);*/
//	TEsT

	image.Create(m_width,m_height,32);
	m_d3dTexture.Create(D3DKernel::GetD3DDevice(),&image,0,0);
	if(m_d3dTexture.Surface())
	{
		//	выравниваем текстурные координаты
		m_TexCoord[0] = texcoord(0,0);
		m_TexCoord[1] = texcoord((float)m_width/m_d3dTexture.Width(),0);
		m_TexCoord[2] = texcoord((float)m_width/m_d3dTexture.Width(),(float)m_height/m_d3dTexture.Height());
		m_TexCoord[3] = texcoord(0,(float)m_height/m_d3dTexture.Height());
	}
	//	устанавливаем кординаты для треугольников
	for(int i=0;i<4;i++)
		m_d3dTLVertex[i] = D3DTLVERTEX(D3DVECTOR(m_Vertex[i].x,m_Vertex[i].y,m_Vertex[i].z),1,m_dwTextColor,0.0f,m_TexCoord[i].u,m_TexCoord[i].v);
}

void elText::MoveTo(int x,int y)
{
	Lagoon::MoveTo(x,y);
	//	устанавливаем кординаты для треугольников
	for(int i=0;i<4;i++)
		m_d3dTLVertex[i] = D3DTLVERTEX(D3DVECTOR(m_Vertex[i].x,m_Vertex[i].y,m_Vertex[i].z),1,m_dwTextColor,0.0f,m_TexCoord[i].u,m_TexCoord[i].v);
}

void elText::SetFont(const char *pFont)
{
	m_sFont = pFont;
}

void elText::SetText(const char *pText)
{
	m_sText = pText;
}

void elText::AddText(const char *pText)
{
	m_sText += pText;
}

void elText::SetTextColor(DWORD dwTextColor)
{
	m_dwTextColor = dwTextColor;
}

void elText::SetTextHAlign(TEXT_HALIGN ha)
{
	m_uiHAlign = ha;
}

void elText::SetTextVAlign(TEXT_VALIGN va)
{
	m_uiVAlign = va;
}

void elText::TextBlit(void)
{
	ClearSurface();
	FontMgr::Instance()->GetGFont(m_sFont.c_str())->TextOut(m_d3dTexture.Surface(),&m_fText,
		m_iOffsetY,m_width,m_height);
//	TEsT
/*	m_vTexture.Clear();
	FontMgr::Instance()->GetGFont(m_sFont.c_str()).TextOut(&m_vTexture,&m_fText,
		m_iOffsetY,m_width,m_height);*/
//	TEsT
}

void elText::ClearSurface(void)
{
	if(m_d3dTexture.Surface())
	{
		DDSURFACEDESC2 ddsd;
		memset(&ddsd,0,sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		m_d3dTexture.Surface()->GetSurfaceDesc(&ddsd);
		DDBLTFX ddfx;
		memset(&ddfx,0,sizeof(DDBLTFX));
		ddfx.dwSize = sizeof(DDBLTFX);
		ddfx.dwFillColor = ddsd.ddpfPixelFormat.dwRBitMask|
						   ddsd.ddpfPixelFormat.dwGBitMask|
						   ddsd.ddpfPixelFormat.dwBBitMask;
		//	очищаем поверхность
		m_d3dTexture.Surface()->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&ddfx);
	}
}

void elText::Render(GraphPipe * /*lpGraphPipe*/)
{
	if(m_bVisible)
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
		StatesManager::SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
		StatesManager::SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
		StatesManager::SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
		StatesManager::SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,0);

//	TEsT
//	m_vTexture.Render();
//	TEsT

		D3DKernel::GetD3DDevice()->DrawPrimitive(D3DPT_TRIANGLEFAN,D3DFVF_TLVERTEX,
											 m_d3dTLVertex,4,0*D3DDP_WAIT);
	}
}
