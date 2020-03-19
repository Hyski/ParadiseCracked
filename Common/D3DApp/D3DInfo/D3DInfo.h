/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
        Информация о доступных устройствах и их режимах.
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   16.07.2000

************************************************************************/
#ifndef _D3D_INFO_H_
#define _D3D_INFO_H_

/*************************** class VideoMode *************************/
class VideoMode
{
public:
	unsigned int width;
	unsigned int height;   
	unsigned int bpp;  
	unsigned int freq;
public:
	VideoMode()
	{
		width = 0; height = 0; bpp = 0; freq = 0;
	};
	VideoMode(unsigned int w,unsigned int h,unsigned int b,unsigned int f)
	{
		width = w; height = h; bpp = b; freq = f;
	};
};

inline bool operator == (const VideoMode& x,const VideoMode& y)
{
	if((x.width==y.width) && (x.height==y.height) && (x.bpp==y.bpp) && (x.freq==y.freq))
		return true;
	return false;
}

inline bool operator < (const VideoMode& x,const VideoMode& y)
{
	if(x.width < y.width)
		return true;
	if(x.width > y.width)
		return false;
	if(x.height < y.height)
		return true;
	if(x.height > y.height)
		return false;
	if(x.freq < y.freq)
		return true;
	if(x.freq > y.freq)
		return false;
	if(x.bpp < y.bpp)
		return true;

	return false;
}

/*************************** class DeviceInfo *************************/
class DeviceInfo
{
public:
	std::string sDescription;		//	название девайса
	std::string sName;				//	название девайса
	GUID guid;						//	глобальный индефикатор
	bool bHardware;					//	флаг поддержки HAL
	bool bTransformAndLighting;		//	флаг поддержки Трансформации и Света
	std::vector<VideoMode> vMode;
};

inline bool operator < (const DeviceInfo& x,const DeviceInfo& y)
{
	if(x.bTransformAndLighting && x.bHardware)
		return true;
	if(y.bTransformAndLighting && y.bHardware)
		return false;
	if(x.bHardware)
		return true;
	if(y.bHardware)
		return false;
	if(x.bTransformAndLighting)
		return true;
	if(y.bTransformAndLighting)
		return false;
	
	return false;
}

/*************************** struct DriverInfo *************************/
struct DriverInfo 
{
	std::string sDescription;		//	название драйвера
	std::string sName;				//	название драйвера
	GUID guid;						//	уникальный идентификатор драйвера
	bool bPrimary;					//	флаг первичного драйвера
	DDCAPS	ddDriverCaps;			//	driver capabilities
	DDCAPS	ddHELCaps;				//	hardware emulation level capabilities
	unsigned long ulMonitorFreq;		//	текущая частота регенерации монитора
	std::vector<VideoMode> vMode;		//	доступные видеорежимы
	std::vector<DeviceInfo> vDevice;	//	вектор доступных d3dDevice'ов и информация о них
};
/*************************************************************************/

namespace D3DInfo
{
	extern std::vector<DriverInfo> m_vDriverInfo;
	//	-------------------------------------------------------------
	void Refresh(void);
	void CheckIndex(unsigned int *pDriver,unsigned int *pDevice);
	GUID* GetDriverGuidPtr(unsigned int iDriver);
	GUID* GetDeviceGuidPtr(unsigned int iDriver,unsigned int iDevice);
}

#endif	//_D3D_INFO_H_