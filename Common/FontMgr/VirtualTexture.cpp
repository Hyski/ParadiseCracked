/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   25.09.2000

************************************************************************/
#include "Precomp.h"
#include "../../globals.h"
#include "../TextureMgr/DIBData.h"
#include "VirtualTexture.h"


//---------- Ћог файл ------------
#ifdef _DEBUG_INTERFACE
CLog vt_log;
#define vt	vt_log["vt.log"]
#else
#define vt	/##/
#endif
//--------------------------------

VirtualTexture::VirtualTexture()
{
	m_dwVertexColor = 0xffffff;
}

VirtualTexture::~VirtualTexture()
{
	Release();
}

//	виртуальные координаты
void VirtualTexture::Create(int vWidth,int vHeight)
{
	int maxTexWidth;
	int maxTexHeight;
	int numX = 0;
	int numY = 0;
	int texWidth;
	int texHeight;
	DIBData image;
	D3DTexture tex;
	OnePart op;
	std::vector<OnePart> vParts;

	Release();
	vt("\n--- Create: <%d,%d> ---\n",vWidth,vHeight);
	image.Create(vWidth,vHeight,32);
	tex.Create(D3DKernel::GetD3DDevice(),&image,0,0);
	if(tex.Surface())
	{//	текстура создалась
		maxTexWidth = tex.Width();
		maxTexHeight = tex.Height();
		vt("    ...max sizes: [%d,%d];\n",maxTexWidth,maxTexHeight);
		//	провер€ем достаточность
		if((maxTexWidth!=0) || (maxTexHeight!=0))
		{
			numX = (vWidth/maxTexWidth)+((vWidth%maxTexWidth)?1:0);
			numY = (vHeight/maxTexHeight)+((vHeight%maxTexHeight)?1:0);
			vt("   ...num of textures: [%d,%d]\n",numX,numY);
			for(int i=0;i<numY;i++)
			{
				//	--------------------------------------------------------
				//	рассчитываем размеры текстуры
				if((maxTexHeight*(i+1))<vHeight)
					texHeight = maxTexHeight;
				else
					texHeight = vHeight-(maxTexHeight*i);
//					texHeight = (maxTexHeight*(i+1))-vHeight;
				vt(">> ------- Row #%d ------- <<\n",i);
				for(int j=0;j<numX;j++)
				{
					vt("    ...trying to create texture:\n");
					//	создаем новую текстуру
					op.m_pTexture = new D3DTexture();
					if(!op.m_pTexture)
						CASUS("Ќе удалось выделить достаточное кол-во пам€ти под объект класса D3DTexture.\n");
					//	--------------------------------------------------------
					//	рассчитываем размеры текстуры
					if((maxTexWidth*(j+1))<vWidth)
						texWidth = maxTexWidth;
					else
						texWidth = vWidth-(maxTexWidth*j);
//						texWidth = (maxTexWidth*(j+1))-vWidth;
					vt("    - size for texture: [%d,%d];\n",texWidth,texHeight);
					//	--------------------------------------------------------
					image.Create(maxTexWidth,maxTexHeight,32);
					op.m_pTexture->Create(D3DKernel::GetD3DDevice(),&image,0,0);
					vt("    - real size of texture: [%d,%d];\n",op.m_pTexture->Width(),op.m_pTexture->Height());
					//	--------------------------------------------------------
					//	рассчитываем данные дл€ отображени€
					CalcRenderData(&op,
						ToScreenX(texWidth),ToScreenY(texHeight),
						ToScreenX(j*maxTexWidth),ToScreenY(i*maxTexHeight),
						(float)texWidth/op.m_pTexture->Width(),
						(float)texHeight/op.m_pTexture->Height());
//					op.m_d3dTLVertex[0] = D3DTLVERTEX(D3DVECTOR(j*maxTexWidth,i*maxTexHeight.y,0),1,m_dwTextColor,0.0f,0,0);


					//	добавл€ем часть текстуры
					vParts.push_back(op);		
					vt("     - creating DONE.\n");
				}
				//	добавл€ем массив частей текстуры
				m_TexMatrix.push_back(vParts);
				vParts.clear();
			}
		}
	}
}

void VirtualTexture::Release(void)
{
	for(int i=0;i<m_TexMatrix.size();i++)
	{
		for(int j=0;j<m_TexMatrix[i].size();j++)
		{
			if(m_TexMatrix[i][j].m_pTexture)
				delete m_TexMatrix[i][j].m_pTexture;
		}
	}
	m_TexMatrix.clear();
}

//*******************************************************************************//
void VirtualTexture::Clear(void)
{
	for(int i=0;i<m_TexMatrix.size();i++)
	{
		for(int j=0;j<m_TexMatrix[i].size();j++)
		{
			DDSURFACEDESC2 ddsd;
			memset(&ddsd,0,sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			m_TexMatrix[i][j].m_pTexture->Surface()->GetSurfaceDesc(&ddsd);
			DDBLTFX ddfx;
			memset(&ddfx,0,sizeof(DDBLTFX));
			ddfx.dwSize = sizeof(DDBLTFX);
			ddfx.dwFillColor = ddsd.ddpfPixelFormat.dwRBitMask|
				ddsd.ddpfPixelFormat.dwGBitMask|
				ddsd.ddpfPixelFormat.dwBBitMask;
			//	очищаем поверхность
			m_TexMatrix[i][j].m_pTexture->Surface()->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&ddfx);
		}
	}
}

void VirtualTexture::Render(void)
{
	for(int i=0;i<m_TexMatrix.size();i++)
	{
		for(int j=0;j<m_TexMatrix[i].size();j++)
		{
			D3DKernel::GetD3DDevice()->SetTexture(0,m_TexMatrix[i][j].m_pTexture->Surface());
			D3DKernel::GetD3DDevice()->DrawPrimitive(D3DPT_TRIANGLEFAN,
													 D3DFVF_TLVERTEX,
													 m_TexMatrix[i][j].m_d3dTLVertex,
													 4,
													 D3DDP_WAIT);
		}
	}
}

void VirtualTexture::Blt(LPRECT pDestRect,LPDIRECTDRAWSURFACE7 pDDSrcSurface,  
						 LPRECT pSrcRect,DWORD dwFlags,LPDDBLTFX pDDBltFx)
{
	RECT lDstRect = {0,0,50,50};
//	RECT lSrcRect;
	POINT pt;
	int x=0,y=0;
	bool  bBelong[4];		

	for(int i=0;i<m_TexMatrix.size();i++)	//	строчки
	{
		for(int j=0;j<m_TexMatrix[i].size();j++)	//	столбцы
		{
			pt.x = x; pt.y = y;
			bBelong[0] = PtInRect(pDestRect,pt);
			pt.x = x+m_TexMatrix[i][j].m_pTexture->Width();
			bBelong[1] = PtInRect(pDestRect,pt);
			pt.y = y+m_TexMatrix[i][j].m_pTexture->Height();
			bBelong[2] = PtInRect(pDestRect,pt);
			pt.x = x; 
			bBelong[3] = PtInRect(pDestRect,pt);
			if(bBelong[0] || bBelong[1] || bBelong[2] || bBelong[3])
			{
				//	--------------------------------
				if(bBelong[0] || bBelong[1]) lDstRect.left = x;
				else lDstRect.left = pDestRect->left;
				if(bBelong[1] || bBelong[2]) lDstRect.top = y;
				else lDstRect.top = pDestRect->top;
				if(bBelong[2] || bBelong[3]) lDstRect.right = x+m_TexMatrix[i][j].m_pTexture->Width();
				else lDstRect.right = pDestRect->right;
				if(bBelong[3] || bBelong[0]) lDstRect.bottom = y+m_TexMatrix[i][j].m_pTexture->Height();
				else lDstRect.bottom = pDestRect->bottom;
				//	--------------------------------
				m_TexMatrix[i][j].m_pTexture->Surface()->Blt(&lDstRect,pDDSrcSurface,pSrcRect,dwFlags,pDDBltFx);
			}
			x += m_TexMatrix[i][j].m_pTexture->Width();
		}
		if(m_TexMatrix[i].size())
			y += m_TexMatrix[i][0].m_pTexture->Height();
	}
	
}

//*******************************************************************************//
void VirtualTexture::CalcRenderData(OnePart *pOP,float width,float height,float x,float y,float texWidth,float texHeight)
{
	vt("\n--- Calc Render Data ---\n");
	vt("    ...width:height - [%0.4f:%0.4f]\n",width,height);
	vt("    ...x:y - [%0.4f:%0.4f]\n",x,y);
	vt("    ...texWidth:texHeight - [%0.4f:%0.4f]\n",texWidth,texHeight);
	pOP->m_d3dTLVertex[0] = D3DTLVERTEX(D3DVECTOR(x,y,0),1,m_dwVertexColor,0.0f,0,0);
	pOP->m_d3dTLVertex[1] = D3DTLVERTEX(D3DVECTOR(x+width,y,0),1,m_dwVertexColor,0.0f,texWidth,0);
	pOP->m_d3dTLVertex[2] = D3DTLVERTEX(D3DVECTOR(x+width,y+height,0),1,m_dwVertexColor,0.0f,texWidth,texHeight);
	pOP->m_d3dTLVertex[3] = D3DTLVERTEX(D3DVECTOR(x,y+height,0),1,m_dwVertexColor,0.0f,0,texHeight);
}

