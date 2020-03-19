/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
        Информация о доступных устройствах и ихрежимах.
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   16.07.2000

************************************************************************/
#include "Precomp.h"
#include "D3DInfo.h"

#ifdef _ENABLE_LOG
CLog d3dinfo_log;
#define d3dinfo	d3dinfo_log["d3dinfo.log"]
#else
#define d3dinfo	/##/
#endif

namespace D3DInfo
{
	std::vector<DriverInfo> m_vDriverInfo;
}

namespace D3DInfo
{
	BOOL WINAPI DriverEnumCallback(GUID* lpGUID,
								   LPSTR lpDriverDescription,
								   LPSTR lpDriverName,
								   VOID* lpContext,
								   HMONITOR hm);
	HRESULT WINAPI ModeEnumCallback(LPDDSURFACEDESC2 lpDDSurfaceDesc,
									LPVOID lpContext);
	HRESULT CALLBACK DeviceEnumCallback(LPSTR lpDeviceDescription, 
										LPSTR lpDeviceName,
										LPD3DDEVICEDESC7 lpD3DDeviceDesc,
										LPVOID lpContext);
}

void D3DInfo::Refresh(void)
{
	m_vDriverInfo.clear();
	d3dinfo("--- Refresh Direct3D hardware information ---\n");
	DirectDrawEnumerateEx(DriverEnumCallback,
						  0, 
						 DDENUM_ATTACHEDSECONDARYDEVICES |
						 DDENUM_DETACHEDSECONDARYDEVICES |
						 DDENUM_NONDISPLAYDEVICES);	

#ifdef _ENABLE_LOG
	d3dinfo("\n--- Sorted data ---\n\n");
	for(int i=0;i<m_vDriverInfo.size();i++)
	{
		d3dinfo("Driver - <%s> -\n",m_vDriverInfo[i].sDescription.c_str());
		d3dinfo("     ...current monitor frequency: %ld\n",m_vDriverInfo[i].ulMonitorFreq);
		d3dinfo("     ...available video modes:\n");
		for(int j=0;j<m_vDriverInfo[i].vMode.size();j++)
		{
			d3dinfo("      - %dx%dx%dx%d\n",m_vDriverInfo[i].vMode[j].width,m_vDriverInfo[i].vMode[j].height,m_vDriverInfo[i].vMode[j].bpp,m_vDriverInfo[i].vMode[j].freq);
		}
		for(j=0;j<m_vDriverInfo[i].vDevice.size();j++)
		{
			d3dinfo("     ...Device - <%s>\n",m_vDriverInfo[i].vDevice[j].sDescription.c_str());
			d3dinfo("        ...available video modes:\n");
			for(int k=0;k<m_vDriverInfo[i].vDevice[j].vMode.size();k++)
			{
				d3dinfo("         - %dx%dx%dx%d\n",m_vDriverInfo[i].vDevice[j].vMode[k].width,m_vDriverInfo[i].vDevice[j].vMode[k].height,m_vDriverInfo[i].vDevice[j].vMode[k].bpp,m_vDriverInfo[i].vDevice[j].vMode[k].freq);
			}
		}
	}
	d3dinfo("\n");
#endif	//_ENABLE_LOG
}

BOOL WINAPI D3DInfo::DriverEnumCallback(GUID* lpGUID,
										LPSTR lpDriverDescription,
										LPSTR lpDriverName,
										VOID* ,
										HMONITOR )
{
	DriverInfo di;
    LPDIRECTDRAW7 lpDD = NULL;
    LPDIRECT3D7   lpD3D = NULL;

	d3dinfo("    ...find driver:\n",lpDriverDescription);
	//	запоминаем названия дравера
	di.sDescription = lpDriverDescription;
	di.sName = lpDriverName;
	d3dinfo("     - driver description (\"%s\");\n",lpDriverDescription);
	d3dinfo("     - driver name (\"%s\");\n",lpDriverName);

    // Use the GUID to create the DirectDraw object
    if(FAILED(DirectDrawCreateEx(lpGUID,(VOID**)&lpDD,IID_IDirectDraw7,NULL)))
        return DDENUMRET_OK;
	//	запоминаем уникальный идентификатор драйвера
	if(lpGUID)
	{
		di.guid = (*lpGUID);
		di.bPrimary = false;
	}
	else
	{
		di.bPrimary = true;
	}
	//	Получение свойств для HARDWARE и HARDWARE EMULATION LEVEL
	di.ddDriverCaps.dwSize = sizeof(DDCAPS); 
	di.ddHELCaps.dwSize = sizeof(DDCAPS); 
    lpDD->GetCaps(&di.ddDriverCaps,&di.ddHELCaps);
    // Add a display mode for rendering into a desktop window
    if(di.bPrimary &&
       (di.ddDriverCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED))
    {
        // Get the current display depth
        DEVMODE devmode;
        devmode.dmSize = sizeof(DEVMODE);
        EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&devmode);
        // Set up the mode info
		di.vMode.push_back(VideoMode(0,0,devmode.dmBitsPerPel,devmode.dmDisplayFrequency ));
    }
	//	Enumerate the fullscreen display modes.
	lpDD->EnumDisplayModes(DDEDM_REFRESHRATES,NULL,&di,ModeEnumCallback);
    // Create a D3D object, to enumerate the d3d devices
	if(FAILED(lpDD->QueryInterface(IID_IDirect3D7,(VOID**)&lpD3D)))
	{
        lpDD->Release();
	    return D3DENUMRET_OK;
	}
	//	Now, enumerate all the 3D devices:
    lpD3D->EnumDevices(DeviceEnumCallback,&di);
	//	определяем частоту регенирации монитора
	if(FAILED(lpDD->GetMonitorFrequency(&di.ulMonitorFreq)))
		di.ulMonitorFreq = 0;
	//	сортируем
	std::sort(di.vMode.begin(),di.vMode.end());
	std::sort(di.vDevice.begin(),di.vDevice.end());
	//	запоминаем
	m_vDriverInfo.push_back(di);

	lpD3D->Release();
    lpDD->Release();

	return DDENUMRET_OK;
}

HRESULT WINAPI D3DInfo::ModeEnumCallback(LPDDSURFACEDESC2 lpDDSurfaceDesc,
										 LPVOID lpContext)
{
	DriverInfo *lpDI = (DriverInfo *)lpContext;
    // Copy the mode into the driver's list of supported modes
	lpDI->vMode.push_back(VideoMode(lpDDSurfaceDesc->dwWidth,
								 lpDDSurfaceDesc->dwHeight,
								 lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount,
								 lpDDSurfaceDesc->dwRefreshRate));

    return DDENUMRET_OK;
}

HRESULT CALLBACK D3DInfo::DeviceEnumCallback(LPSTR lpDeviceDescription, 
											 LPSTR lpDeviceName,
											 LPD3DDEVICEDESC7 lpD3DDeviceDesc,
											 LPVOID lpContext)
{
	DriverInfo *lpDI = (DriverInfo *)lpContext;
	DeviceInfo di;

	//	запоминаем имена
	di.sDescription = lpDeviceDescription;
	di.sName = lpDeviceName;
	di.guid = lpD3DDeviceDesc->deviceGUID;
	d3dinfo("     - find device:\n");
	d3dinfo("          ...description (\"%s\");\n",lpDeviceDescription);
	d3dinfo("          ...name (\"%s\");\n",lpDeviceName);
	d3dinfo("          ...guidDevice == {%x-%x-%x-...};\n",lpD3DDeviceDesc->deviceGUID.Data1,lpD3DDeviceDesc->deviceGUID.Data2,lpD3DDeviceDesc->deviceGUID.Data3);
	d3dinfo("          ...min & max texture size: (%dx%d)-(%dx%d);\n",lpD3DDeviceDesc->dwMinTextureWidth,lpD3DDeviceDesc->dwMinTextureHeight,lpD3DDeviceDesc->dwMaxTextureWidth,lpD3DDeviceDesc->dwMaxTextureHeight);

	//	запоминаем свойства
	di.bHardware = lpD3DDeviceDesc->dwDevCaps&D3DDEVCAPS_HWRASTERIZATION;
	d3dinfo("          ...D3DDEVCAPS_HWRASTERIZATION     %s;\n",di.bHardware?"present":"absent");
	di.bTransformAndLighting = lpD3DDeviceDesc->dwDevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT;
	d3dinfo("          ...D3DDEVCAPS_HWTRANSFORMANDLIGHT %s;\n",di.bTransformAndLighting?"present":"absent");

    // Build list of supported modes for the device
    for(int i=0;i<lpDI->vMode.size();i++)
    {
        bool bCompatible = false;

        // Check mode for compatability with device. Skip 8-bit modes.
        if((32==lpDI->vMode[i].bpp) && (lpD3DDeviceDesc->dwDeviceRenderBitDepth&DDBD_32)) bCompatible = true;
        if((24==lpDI->vMode[i].bpp) && (lpD3DDeviceDesc->dwDeviceRenderBitDepth&DDBD_24)) bCompatible = true;
        if((16==lpDI->vMode[i].bpp) && (lpD3DDeviceDesc->dwDeviceRenderBitDepth&DDBD_16)) bCompatible = true;

        // Copy compatible modes to the list of device-supported modes
		if(bCompatible)
			di.vMode.push_back(lpDI->vMode[i]);
    }

	//	сортируем
	std::sort(di.vMode.begin(),di.vMode.end());
	//	запоминаем
	lpDI->vDevice.push_back(di);

	return D3DENUMRET_OK;
}

void D3DInfo::CheckIndex(unsigned int *pDriver,unsigned int *pDevice)
{
	if(m_vDriverInfo.size())
	{
		if(*pDriver>=m_vDriverInfo.size())
			*pDriver = m_vDriverInfo.size()-1;
		if(*pDevice>=m_vDriverInfo[*pDriver].vDevice.size())
			*pDevice = m_vDriverInfo[*pDriver].vDevice.size()-1;
	}
}

GUID* D3DInfo::GetDriverGuidPtr(unsigned int iDriver)
{
	static GUID guid;

	if(iDriver<m_vDriverInfo.size())
		if(!m_vDriverInfo[iDriver].bPrimary)
			return &(guid = m_vDriverInfo[iDriver].guid);

	return 0;
}

GUID* D3DInfo::GetDeviceGuidPtr(unsigned int iDriver,unsigned int iDevice)
{
	static GUID guid;

	if(iDriver<m_vDriverInfo.size())
		if(iDevice<m_vDriverInfo[iDriver].vDevice.size())
			return &(guid = m_vDriverInfo[iDriver].vDevice[iDevice].guid);

	return 0;
}
