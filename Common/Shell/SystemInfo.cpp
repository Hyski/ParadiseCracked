/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   31.01.2001

************************************************************************/
#include "Precomp.h"
#include "../Timer/Timer.h"
#include "../D3DApp/D3DInfo/D3DInfo.h"
#include <lmcons.h>
#include "CpuInfo.h"
#include "SystemInfo.h"

namespace SystemInfo
{
	CLConsole m_log_file;
	//	-----------------------
	void GetSystemVersion(void);
	void GetDirectXInfo(void);
	void GetDeviceID(GUID *g);
}

void SystemInfo::Init(void)
{
	struct tm* system_time = Timer::GetSysTime();
	SYSTEM_INFO si;
	char pComputerName[MAX_COMPUTERNAME_LENGTH+1];
	char pUserName[UNLEN+1];
	unsigned long ulSize;

	m_log_file.Open("system.info");

	m_log_file("Game: Paradise Cracked\n\n");
	m_log_file("Date: %0.2d.%0.2d.%0.2d\n",system_time->tm_mday,system_time->tm_mon+1,1900+system_time->tm_year);
	m_log_file("Time: %0.2d:%0.2d:%0.2d\n",system_time->tm_hour,system_time->tm_min,system_time->tm_sec);

	m_log_file("\nSystem Information:\n\n");
	GetSystemVersion();
	ulSize = sizeof(pComputerName);
	GetComputerName(pComputerName,&ulSize);
	m_log_file("\nComputer name: [%s]\n",pComputerName);
	ulSize = sizeof(pUserName);
	GetComputerName(pUserName,&ulSize);
	m_log_file("    User name: [%s]\n",pUserName);

	GetSystemInfo(&si);
	m_log_file("\nNumber of processors: %d\n",si.dwNumberOfProcessors);

	//	информация о процессоре
	CpuInfo ci;

	m_log_file("\nCPU Information:\n\n");
	if(ci.m_bSupport)
	{
		m_log_file("Vendor:   %s\n",ci.m_sVendor);
		m_log_file("CPU Name: %s\n",ci.m_sCpuName);
		m_log_file("--- Signature ---\n");
		m_log_file("Type:     %d\n",ci.m_iType);
		m_log_file("Family:   %d\n",ci.m_iFamily);
		m_log_file("Model:    %d\n",ci.m_iModel);
		m_log_file("Stepping: %d\n",ci.m_iStepping);
		m_log_file("--- Features ---\n");
		m_log_file("FPU:      %s\n",ci.m_bFPU?"present":"absent");
		m_log_file("MMX:      %s\n",ci.m_bMMX?"present":"absent");
		m_log_file("3DNow!:   %s\n",ci.m_b3DNow?"present":"absent");
		m_log_file("SIMD:     %s\n",ci.m_bSIMD?"present":"absent");
		m_log_file("CMOV:     %s\n",ci.m_bCMOV?"present":"absent");
		m_log_file("TSC:      %s\n",ci.m_bTSC?"present":"absent");
	}
	else
	{
		m_log_file("... function failed;\n");
	}
	//	информация о DirectX
	GetDirectXInfo();
}

void SystemInfo::GetSystemVersion(void)
{
	DWORD dwVersion;
	OSVERSIONINFOEX osvi;
	int iBuildNumber = 0;

	//	получаем общую информацию о версии системы
	dwVersion = GetVersion();
	if(dwVersion < 0x80000000)	//	Windows NT
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	else						//	Other Windows
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	//	получаем более подробную информацию
	if(GetVersionEx((OSVERSIONINFO *)&osvi))
	{
		m_log_file("Platform: ");
		switch(osvi.dwPlatformId)
		{
		case VER_PLATFORM_WIN32s:
			m_log_file("Windows 3.1\n");
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			if(((osvi.dwMajorVersion > 4) ||
			   ((osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion > 0))))
			{
				m_log_file("Windows 98 or later\n");
				iBuildNumber = LOWORD(osvi.dwBuildNumber);
			}
			else
			{
				m_log_file("Windows 95\n");
			}
			break;
		case VER_PLATFORM_WIN32_NT:
			m_log_file("Windows NT\n");
			iBuildNumber = osvi.dwBuildNumber;
			if((osvi.wServicePackMajor!=0) || (osvi.wServicePackMinor!=0))
				m_log_file("Service Pack:    %d.%d\n",osvi.wServicePackMajor,osvi.wServicePackMinor);

			break;
		}
	}
	else
	{
		m_log_file("... function failed;\n");
		return;
	}
	m_log_file("Verison:  %d.%d\n",osvi.dwMajorVersion,osvi.dwMinorVersion);
	if(iBuildNumber)
		m_log_file("Build:    %d\n",iBuildNumber);
}

void SystemInfo::GetDirectXInfo(void)
{
	D3DInfo::Refresh();

	m_log_file("\nDirectX Information:\n\n");

	if(!D3DInfo::m_vDriverInfo.size())
	{
		m_log_file("... function failed;\n");
	}
	else
	{
		for(int i=0;i<D3DInfo::m_vDriverInfo.size();i++)
		{
			m_log_file("    Driver: %s\n",D3DInfo::m_vDriverInfo[i].sDescription.c_str());
			GetDeviceID(D3DInfo::GetDriverGuidPtr(i));
			for(int j=0;j<D3DInfo::m_vDriverInfo[i].vDevice.size();j++)
			{
				m_log_file("\n        Device: %s\n",D3DInfo::m_vDriverInfo[i].vDevice[j].sDescription.c_str());
//				m_log_file("        min & max texture size: [%d]:[%d]\n",D3DInfo::m_vDriverInfo[i].vDevice[j].sDescription.c_str());
			}
		}
	}
}

void SystemInfo::GetDeviceID(GUID *g)
{
	DDDEVICEIDENTIFIER2 dddi;
	LPDIRECTDRAW7 lpDD;	

	if(!FAILED(DirectDrawCreateEx(g,(VOID**)&lpDD,IID_IDirectDraw7,NULL)))
	{
		memset(&dddi,0,sizeof(DDDEVICEIDENTIFIER2));
		lpDD->GetDeviceIdentifier(&dddi,DDGDI_GETHOSTIDENTIFIER);
		m_log_file("\n     driver name:        [%s]\n",dddi.szDriver);
		m_log_file("     driver description: [%s]\n",dddi.szDescription);
		m_log_file("     product:     %d\n",HIWORD(dddi.liDriverVersion.HighPart));
		m_log_file("     verison:     %d\n",LOWORD(dddi.liDriverVersion.HighPart));
		m_log_file("     sub version: %d\n",HIWORD(dddi.liDriverVersion.LowPart));
		m_log_file("     build:       %d\n",LOWORD(dddi.liDriverVersion.LowPart));

		//	освобождаем объект DirectDraw
		lpDD->Release();
	}
	else
	{
		m_log_file("...function failed;\n");
	}
}
