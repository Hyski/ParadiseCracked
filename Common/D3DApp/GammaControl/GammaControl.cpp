/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   05.07.2000

************************************************************************/
#include "Precomp.h"
#include "GammaControl.h"

#define max(a,b)	(a>b?a:b)
#define min(a,b)	(a<b?a:b)

namespace GammaControl
{
	LPDIRECTDRAWGAMMACONTROL m_pDDGC = 0;
	DDGAMMARAMP m_GammaRamp;
	//	---------------------------------
	int m_iBrightness = 0;
	int m_iBrightnessStep = 1000;
	float m_fGamma = 1;
	float m_fGammaStep = 0.07;
	int m_iContrast = 256;
	int m_iContrastStep = 10;
}

namespace GammaControl
{
	void SetGammaControl(void);
}

void GammaControl::Init(LPDIRECTDRAWSURFACE7 pPrimarySurface)
{
	RELEASE(m_pDDGC);
	pPrimarySurface->QueryInterface(IID_IDirectDrawGammaControl,(LPVOID*)&m_pDDGC);
	SetGammaControl();
}

void GammaControl::Release(void)
{
	RELEASE(m_pDDGC);
}

void GammaControl::SetGammaControl(void)
{
	if(m_pDDGC)
	{
/*		for(int i=(-128);i<128;i++)
		{
			m_GammaRamp.red[i+128]		= 
			m_GammaRamp.green[i+128]	= 
			m_GammaRamp.blue[i+128]		= saturate((int)(((i*m_iContrast)+m_iBrightness)+(65535>>1)),0,65535);
		}*/
		for(int i=0;i<256;i++)
		{
//			float f = pow((float)i/256,1.f/((float)m_iBrightness/10000))*65535;
			float c = saturate((float)(i-128)*((float)m_iContrast/256)+128,0.f,255.f);
			float f = saturate((float)pow((float)c/255,1.f/m_fGamma)*65535+m_iBrightness,0.f,65535.f);
//			m_fGamma
//			f += 
			m_GammaRamp.red[i]		= 
			m_GammaRamp.green[i]	= 
			m_GammaRamp.blue[i]		= f;
		}
		m_pDDGC->SetGammaRamp(0,&m_GammaRamp);
	}
}

void GammaControl::IncBrightness(void)
{
	m_iBrightness = min(65535,m_iBrightness+m_iBrightnessStep);
	SetGammaControl();
}

void GammaControl::DecBrightness(void)
{
	m_iBrightness = max(-65535,m_iBrightness-m_iBrightnessStep);
	SetGammaControl();
}

void GammaControl::IncContrast(void)
{
	m_iContrast = min(65535,m_iContrast+m_iContrastStep);
	SetGammaControl();
}

void GammaControl::DecContrast(void)
{
	m_iContrast = max(0,m_iContrast-m_iContrastStep);
	SetGammaControl();
}

DDGAMMARAMP GammaControl::GammaRamp(void)
{
	return m_GammaRamp;
}

void GammaControl::IncGamma(void)
{
	if((m_fGamma+m_fGammaStep)<6)
		m_fGamma += m_fGammaStep;
	else 
		m_fGamma = 6;
	SetGammaControl();
}

void GammaControl::DecGamma(void)
{
	if((m_fGamma-m_fGammaStep)>0.5)
		m_fGamma -= m_fGammaStep;
	else
		m_fGamma = 0.5;
	SetGammaControl();
}

void GammaControl::SetGamma(float g)
{
	m_fGamma = saturate(g,0.5f,6.f);
	SetGammaControl();
}

float GammaControl::GetGamma(void)
{
	return m_fGamma;
}

