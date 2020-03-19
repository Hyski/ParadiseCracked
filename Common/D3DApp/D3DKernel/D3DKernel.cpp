/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)

************************************************************************/
#include "precomp.h"

HRESULT CALLBACK EnumZBufferCallback(LPDDPIXELFORMAT lpDDPixFmt,LPVOID lpContext);

//	---------- Лог файл ------------
#ifdef _DEBUG_GRAPHICS
CLog d3dkernel_log;
#define d3dkernel	d3dkernel_log["d3dkernel.log"]
#else
#define d3dkernel	/##/
#endif
//	--------------------------------	

typedef void (*LPUPDATEFRAME)(void);

namespace D3DKernel
{
	LPDIRECTDRAW7        m_lpDD = NULL;
	LPDIRECTDRAWSURFACE7 m_lpddsPrimary = NULL;
	LPDIRECTDRAWSURFACE7 m_lpddsBackBuffer = NULL;
	LPDIRECTDRAWSURFACE7 m_lpddsZBuffer = NULL;
	LPDIRECT3D7          m_lpD3D = NULL;
	LPDIRECT3DDEVICE7    m_lpd3dDevice = NULL;
	HWND				 m_hPrevWnd = NULL;
	//	-----------------------------------
	RECT                 m_rcScreenRect = {0,0,0,0};
	SIZE				 m_szSize = {0,0};
	BOOL				 m_bTripleBuffering = false;
	//	-----------------------------------
	LPUPDATEFRAME m_lpUpdateFrame;
//	BOOL m_bFullScreen = FALSE;
	//	------------- ФУНКЦИИ ---------------------
	BOOL Init3DWindow(HWND hWnd,
					  GUID* lpDriverGUID,
					  GUID* lpDeviceGUID,
					  int width,
					  int height);
	BOOL Init3DFullScreen(HWND hWnd,
						  GUID* lpDriverGUID,
						  GUID* lpDeviceGUID,
						  int width,
						  int height,
						  int depth,
						  int freq);
	BOOL AddingZBuffer(GUID* lpDeviceGUID);
}

//	---------------- Инициализация --------------------

BOOL D3DKernel::Init(HWND hWnd,
					 GUID* lpDriverGUID,
					 GUID* lpDeviceGUID,
					 int width,
					 int height,
					 int depth,
					 int freq,
					 bool bWindowed)
{
	Release();
	//	запоминаем новое окно
	m_hPrevWnd = hWnd;
	//	инициализируем режим
	d3dkernel("--- D3DKernel initialization ---\n");
//	d3dkernel("    ...driver guid: {%x-%x-%x-...}\n",lpDriverGUID->Data1,lpDriverGUID->Data2,lpDriverGUID->Data3);
	d3dkernel("    ...driver & device pointers to guid: (%x)-(%x)\n",lpDriverGUID,lpDeviceGUID);
//	d3dkernel("    ...device guid: {%x-%x-%x-...}\n",lpDeviceGUID->Data1,lpDeviceGUID->Data2,lpDeviceGUID->Data3);
	if(bWindowed)
	{//	Windowed mode
		m_lpUpdateFrame = Blt;
		return Init3DWindow(hWnd,lpDriverGUID,lpDeviceGUID,width,height);
	}
	else
	{//	Fullscreen mode
		m_lpUpdateFrame = Flip;
		return Init3DFullScreen(hWnd,lpDriverGUID,lpDeviceGUID,width,height,depth,freq);
	}
}

/*void D3DKernel::SetNormalLevel(void)
{
	//	необходимо переключить окно в нормальный режим если оно в fullscreen'е
	if(m_lpDD && IsFullScreen())
	{
		m_lpDD->SetCooperativeLevel(m_hPrevWnd,DDSCL_NORMAL);
	}
}
*/
void D3DKernel::Release(void)
{
	//	необходимо переключить окно в нормальный режим если оно в fullscreen'е
	if(m_lpDD && IsFullScreen() && m_hPrevWnd)
	{
		if(!FAILED(m_lpDD->SetCooperativeLevel(m_hPrevWnd,DDSCL_NORMAL)))
		{
			m_hPrevWnd = NULL;
			d3dkernel("     RESTORE DDSCL_NORMAL COOPERATIVE LEVEL!!!\n");
		}
	}
	//	освобождаем ресурсы
	d3dkernel("--- Releasing 3D ---\n");
	RELEASE(m_lpd3dDevice);
	d3dkernel("    ...Direct3D Device released;\n");
	RELEASE(m_lpD3D);
	d3dkernel("    ...Direct3D Object released;\n");
	RELEASE(m_lpddsZBuffer);
	d3dkernel("    ...Z-Buffer released;\n");
	RELEASE(m_lpddsBackBuffer);
	d3dkernel("    ...Back Buffer released;\n");
	RELEASE(m_lpddsPrimary);
	d3dkernel("    ...Primary Surface released;\n");
	RELEASE(m_lpDD);
	d3dkernel("    ...DirectDraw Object released;\n");
}

//	---------------- Переключение страниц ---------------------
void D3DKernel::UpdateFrame(void)
{
	m_lpUpdateFrame();
}

void D3DKernel::RestoreSurfaces(void)
{
    // Check/restore the primary surface
    if(m_lpddsPrimary)
	{
        if(m_lpddsPrimary->IsLost())
            m_lpddsPrimary->Restore();
	}
    // Check/restore the back buffer
    if(m_lpddsBackBuffer)
	{
        if(m_lpddsBackBuffer->IsLost())
            m_lpddsBackBuffer->Restore();
	}
    // Check/restore the ZBuffer surface
    if(m_lpddsZBuffer)
	{
        if(m_lpddsZBuffer->IsLost())
            m_lpddsZBuffer->Restore();
	}
}

void D3DKernel::MoveViewPort(int x,int y)
{
	m_rcScreenRect.right = x+(m_rcScreenRect.right-m_rcScreenRect.left); 
	m_rcScreenRect.bottom = y+(m_rcScreenRect.bottom-m_rcScreenRect.top); 
	m_rcScreenRect.left = x; 
	m_rcScreenRect.top = y; 
}

BOOL D3DKernel::AddingZBuffer(GUID* lpDeviceGUID)
{
	DDSURFACEDESC2 ddsd;
	DDPIXELFORMAT ddpfZBuffer;
	
	d3dkernel("=--> CD3DWindow::AddingZBuffer enter;\n");
	m_lpD3D->EnumZBufferFormats(*lpDeviceGUID, 
		                        EnumZBufferCallback,(VOID*)&ddpfZBuffer);

	if(sizeof(DDPIXELFORMAT) != ddpfZBuffer.dwSize)
        return FALSE;

	memset(&ddsd,0,sizeof(DDSURFACEDESC2));
	ddsd.dwSize			=  sizeof(DDSURFACEDESC2);
    ddsd.dwFlags        = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
	ddsd.dwWidth        = m_rcScreenRect.right - m_rcScreenRect.left;
	ddsd.dwHeight       = m_rcScreenRect.bottom - m_rcScreenRect.top;
    memcpy(&ddsd.ddpfPixelFormat,&ddpfZBuffer,sizeof(DDPIXELFORMAT));

	if(IsEqualIID(*lpDeviceGUID,IID_IDirect3DHALDevice) || 
	   IsEqualIID(*lpDeviceGUID,IID_IDirect3DTnLHalDevice))
	{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		d3dkernel("    ...Z-Buffer in DDSCAPS_VIDEOMEMORY;\n");
	}
	else
	{
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		d3dkernel("    ...Z-Buffer in DDSCAPS_SYSTEMMEMORY;\n");
	}
    if(FAILED(m_lpDD->CreateSurface(&ddsd,&m_lpddsZBuffer,NULL)))
		return FALSE;
	d3dkernel("    ...create Z-Buffer - [OK];\n");
    if(FAILED(m_lpddsBackBuffer->AddAttachedSurface(m_lpddsZBuffer)))
		return FALSE;
	d3dkernel("    ...add Z-Buffer to Back Buffer - [OK];\n");

	return TRUE;
}

HRESULT CALLBACK EnumZBufferCallback(LPDDPIXELFORMAT lpDDPixFmt,LPVOID lpContext)
{
	d3dkernel("- calling EnumZBufferCallback;\n");
    if(lpDDPixFmt->dwFlags == DDPF_ZBUFFER)
    {
        memcpy(lpContext,lpDDPixFmt,sizeof(DDPIXELFORMAT)); 
        // Return with D3DENUMRET_CANCEL to end the search.
		d3dkernel("- return D3DENUMRET_CANCEL;\n");
        return D3DENUMRET_CANCEL;
    } 
    // Return with D3DENUMRET_OK to continue the search.
	d3dkernel("- return D3DENUMRET_OK;\n");
    return D3DENUMRET_OK;
}

  

BOOL D3DKernel::Init3DWindow(HWND hWnd,
							 GUID* lpDriverGUID,
							 GUID* lpDeviceGUID,
							 int width,
							 int height)
{
	POINT pt;
	DDSURFACEDESC2 ddsd;
	LPDIRECTDRAWCLIPPER pcClipper;
	d3dkernel("---> enter CInitD3DImWindowed::Init3D();\n");
	d3dkernel("--- Set Windowed Mode ---\n");
	d3dkernel("window size: %dx%d;\n",width,height);

	d3dkernel("--- Trying To Initialization ---\n");
	m_rcScreenRect.left = 0;
	m_rcScreenRect.top = 0;
	m_rcScreenRect.right = m_szSize.cx = width;
	m_rcScreenRect.bottom = m_szSize.cy = height;
	pt.x = m_rcScreenRect.left;
	pt.y = m_rcScreenRect.top;
	ClientToScreen(hWnd,&pt);
	m_rcScreenRect.left = pt.x;
	m_rcScreenRect.top = pt.y;
	pt.x = m_rcScreenRect.right;
	pt.y = m_rcScreenRect.bottom;
	ClientToScreen(hWnd,&pt);
	m_rcScreenRect.right = pt.x;
	m_rcScreenRect.bottom = pt.y;
	if(!FAILED(DirectDrawCreateEx(lpDriverGUID, (VOID**)&m_lpDD,IID_IDirectDraw7,NULL)))
	{
		d3dkernel("    ...create DirectDraw Object - [OK];\n");

		if(!FAILED(m_lpDD->SetCooperativeLevel(hWnd,DDSCL_NORMAL|DDSCL_FPUSETUP)))
		{
			d3dkernel("    ...set cooperative level ( DDSCL_NORMAL|DDSCL_FPUSETUP ) - [OK];\n");
			memset(&ddsd,0,sizeof(DDSURFACEDESC2));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			ddsd.dwFlags        = DDSD_CAPS;
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
			// Create the primary surface.
			if(!FAILED(m_lpDD->CreateSurface(&ddsd,&m_lpddsPrimary,NULL)))
			{
				d3dkernel("    ...create Primary Surface - [OK];\n");
				//	create back buffer
				ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
				ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
				ddsd.dwWidth  = width;
				ddsd.dwHeight = height;
				
				if(!FAILED(m_lpDD->CreateSurface(&ddsd,&m_lpddsBackBuffer,NULL)))
				{
					d3dkernel("    ...create Back Buffer - [OK];\n");
					if(!FAILED(m_lpDD->CreateClipper(0,&pcClipper,NULL)))
					{
						d3dkernel("    ...create Clipper - [OK];\n");
						pcClipper->SetHWnd(0,hWnd);
						m_lpddsPrimary->SetClipper(pcClipper);
						pcClipper->Release();
						if(!FAILED(m_lpDD->QueryInterface(IID_IDirect3D7,(VOID**)&m_lpD3D)))
						{
							d3dkernel("    ...query interface ( IID_IDirect3D7 ) - [OK];\n");
							if(AddingZBuffer(lpDeviceGUID))
							{
								if(!FAILED(m_lpD3D->CreateDevice(*lpDeviceGUID,
									m_lpddsBackBuffer,
									&m_lpd3dDevice)))
								{
									d3dkernel("    ...create Direct3D Device - [OK];\n");
									// Create the viewport
									D3DVIEWPORT7 vp = {0,0,width,height,0.0f,1.0f};
									if(!FAILED(m_lpd3dDevice->SetViewport(&vp)))
									{
										d3dkernel("    ...create viewport - [OK];\n");
										return TRUE;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	d3dkernel("---> leave CInitD3DImWindowed::Init3D() - FALSE;\n");

	return FALSE;
}

BOOL D3DKernel::Init3DFullScreen(HWND hWnd,
								 GUID* lpDriverGUID,
								 GUID* lpDeviceGUID,
								 int width,
								 int height,
								 int depth,
								 int freq)
{
	DDSURFACEDESC2 ddsd;
	d3dkernel("---> enter CInitD3DImFullScreen::Init3D();\n");
	d3dkernel("--- Set Fullscreen Mode ---\n");
	d3dkernel("fullscreen mode: %dx%dx%d;\n",width,height,depth);

	d3dkernel("--- Trying To Initialization ---\n");
	m_rcScreenRect.left = 0;
	m_rcScreenRect.top = 0;
	m_rcScreenRect.right = m_szSize.cx = width;
	m_rcScreenRect.bottom = m_szSize.cy = height;
	d3dkernel("    - lpDriverGUID (%x) DeviceGUID (%x);\n",lpDriverGUID,*lpDeviceGUID);
	if(!FAILED(DirectDrawCreateEx(lpDriverGUID, (VOID**)&m_lpDD,IID_IDirectDraw7,NULL)))
	{
		d3dkernel("    ...create DirectDraw Object - [OK];\n");
	    // Query DirectDraw for access to Direct3D
		if(!FAILED(m_lpDD->QueryInterface(IID_IDirect3D7,(VOID**)&m_lpD3D)))
		{
			d3dkernel("    ...query interface ( IID_IDirect3D7 ) - [OK];\n");
			if(!FAILED(m_lpDD->SetCooperativeLevel(hWnd,DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE|DDSCL_FPUSETUP)))
			{
				d3dkernel("    ...set cooperative level ( DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE|DDSCL_FPUSETUP ) - [OK];\n");
				if(!FAILED(m_lpDD->SetDisplayMode(width,height,depth,freq,0L)))
				{
					ZeroMemory(&ddsd,sizeof(DDSURFACEDESC2));
					ddsd.dwSize = sizeof(DDSURFACEDESC2);
					ddsd.dwFlags           = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
					ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | 
											 DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE;
					ddsd.ddsCaps.dwCaps2 = DDSCAPS2_HINTANTIALIASING; 
					ddsd.dwBackBufferCount = m_bTripleBuffering?2:1;

					// Create the primary surface.
					if(!FAILED(m_lpDD->CreateSurface(&ddsd,&m_lpddsPrimary,NULL)))
					{
						d3dkernel("    ...create Primary Surface - [OK];\n");
						// Get a ptr to the back buffer, which will be our render target
						DDSCAPS2 ddscaps = {DDSCAPS_BACKBUFFER,0,0,0};
						if(!FAILED(m_lpddsPrimary->GetAttachedSurface(&ddscaps,&m_lpddsBackBuffer)))
						{
							d3dkernel("    ...get attached surface: Back Buffer - [OK];\n");
							if(AddingZBuffer(lpDeviceGUID))
							{
								if(!FAILED(m_lpD3D->CreateDevice(*lpDeviceGUID,
									m_lpddsBackBuffer,
									&m_lpd3dDevice)))
								{
									d3dkernel("    ...create Direct3D Device - [OK];\n");
									// Create the viewport
									D3DVIEWPORT7 vp = {0,0,width,height,0.0f,1.0f};
									if(!FAILED(m_lpd3dDevice->SetViewport(&vp)))
									{
										d3dkernel("    ...create viewport - [OK];\n");
										return TRUE;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	d3dkernel("---> enter CInitD3DImFullScreen::Init3D() - FALSE;\n");

	return FALSE;
}

void D3DKernel::Blt(void)
{
	if(m_lpddsPrimary->Blt(&m_rcScreenRect,m_lpddsBackBuffer, 
                        NULL,DDBLT_WAIT,NULL)==DDERR_SURFACELOST)
	{
		d3dkernel("    ...DDERR_SURFACELOST!\n");
		RestoreSurfaces();
	}
}

void D3DKernel::Flip(void)
{
	if(m_lpddsPrimary->Flip(NULL,DDFLIP_WAIT)==DDERR_SURFACELOST)
	{
		d3dkernel("    ...DDERR_SURFACELOST!\n");
		RestoreSurfaces();
	}
}

LPDIRECTDRAW7 D3DKernel::GetDD(void)
{
	return m_lpDD;
}

LPDIRECTDRAWSURFACE7 D3DKernel::GetPS(void)
{
	return m_lpddsPrimary;
}

LPDIRECTDRAWSURFACE7 D3DKernel::GetBB(void)
{
	return m_lpddsBackBuffer;
}

LPDIRECT3D7 D3DKernel::GetD3D(void)
{
	return m_lpD3D;
}

LPDIRECT3DDEVICE7 D3DKernel::GetD3DDevice(void)
{
	return m_lpd3dDevice;
}

int D3DKernel::ResX(void)
{
	return m_szSize.cx;
}

int D3DKernel::ResY(void)
{
	return m_szSize.cy;
}

BOOL D3DKernel::IsFullScreen(void)
{
	return (m_lpUpdateFrame == Flip);
}

void D3DKernel::SetTriple(BOOL bTriple)
{
	m_bTripleBuffering = bTriple;
}
