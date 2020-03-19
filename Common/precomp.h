#ifndef _PRECOMP_H_
#define _PRECOMP_H_

//	дл€ совместимости с DirectX 8.x
#define DIRECTINPUT_VERSION	0x0700


#include <map>
#include <set>
#include <list>
#include <deque>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include <new>
#include <cstdio>

#ifndef _WINDOWS_
    //дл€ убыстрени€ компил€ции выбросим
    //лишнее из windows.h
    #define WIN32_LEAN_AND_MEAN                                 
    #include <windows.h>
	// ”берем эти ужасные макросы, объ€вленные где-то в windows.h
	#ifdef max
	#undef max
	#endif
	#ifdef min
	#undef min
	#endif
#endif

#include <time.h>
#include <dinput.h>
//подключим отладочную кучу
#ifdef _DEBUG
    #include <crtdbg.h>
    #define PUNCH_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
    #define new PUNCH_NEW
#endif //_DEBUG

#include "Exception/Exception.h"

//--- Grom ---
#include "utils/stackguard.h"
//------------

namespace D3DKernel {}
namespace Input {}

#include "DataMgr/DataMgr.h"
#include "saveload/saveload.h"

#include "3d/geometry.h"
#include "Utils/Utils.h"
#include "utils/d3dutil.h"

#include "Log/Log.h"
#include "Timer/Timer.h"
#include "DebugInfo/DebugInfo.h"

#include "D3DApp/input/input.h"
#include "D3DAPP/d3dkernel/d3dkernel.h"

#if !defined(_HOME_VERSION)
//#define _HOME_VERSION
#endif
//#define NO_BLOOD //For German users :-(
//#define USE_SECUROM_TRIGGERS
#endif	//_PRECOMP_H_	