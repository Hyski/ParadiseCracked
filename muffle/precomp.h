#if !defined(__PRECOMP_H_INCLUDED_5992692519605117__)
#define __PRECOMP_H_INCLUDED_5992692519605117__

#if !defined(NOWINBASEINTERLOCK)
//#define NOWINBASEINTERLOCK
#endif

#define NOMINMAX
#define __MUFFLE_EXPORTS__
#define _WIN32_WINNT 0x0400

#include <new>
#include <memory>
#include <string>
#include <sstream>
#include <exception>
#include <assert.h>
#include <hash_map>
#include <map>
#include <algorithm>
#include <vector>
#include <deque>
#include <strstream>
#include <set>
#include <iomanip>
#include <time.h>

#ifdef _DEBUG
    #include <crtdbg.h>
    #define PUNCH_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
    #define new PUNCH_NEW
#endif //_DEBUG

#include <dsound.h>

#include <ISound.h>
#include <VFS.h>
#include <quat.h>		// Paradise Cracked's point3
#include <exception.h>	// Paradise Cracked's CasusImprovisus
#include <stackguard.h>	// Paradise Cracked's StackGuard

#include <mll/debug/log.h>
#include <mll/io/binstream.h>
#include <mll/io/manip.h>
#include <mll/debug/static_assert.h>

using mll::io::ibinstream;

#define __MUFFLE_NO_HARDWARE_BUFFERS__

#if 0&&defined(_DEBUG)
#define KERNEL_LOG(List)	MLL_MAKE_DEBUG_LOG("sound/kernel.log",List)
#define NOFILE_LOG(List)	MLL_MAKE_LOG("sound/file_not_found.log",List)
#else
#define KERNEL_LOG(List)
#define NOFILE_LOG(List)
#endif

// —тратегии завершени€ работы библиотеки

// —тратеги€ 1.
//	if(—уществуютЁмиттеры)
//  {
//		ѕодождатьЌекоторое¬рем€;
//		if(—уществуютЁмиттеры) √рохнуть»х;
//	}
#define MUFFLE_DESTRUCT_STRATEGY			1

// —тратеги€ 2.
//	”ничтожить¬сеЁмиттеры ромећузыки;
//	ѕодождать«авершени€ѕрогрывани€ћузыки;
//#define MUFFLE_DESTRUCT_STRATEGY			2

//#define MUFFLE_WAIT_FOR_MUSIC_FADE_OUT

enum
{
	piece_size = 32768/2,
	piece_count = 8,
	max_buffer_size = piece_size * piece_count,
};

//=====================================================================================//
//                               class CountedInstances                                //
//=====================================================================================//
template<typename T>
class CountedInstances
{
	static long m_instanceCount;

public:
	CountedInstances() { InterlockedIncrement(&m_instanceCount); }
	CountedInstances(const CountedInstances &) { InterlockedIncrement(&m_instanceCount); }
	~CountedInstances() { InterlockedDecrement(&m_instanceCount); }

	static long getInstanceCount() { return m_instanceCount; }
};

template<typename T>
long CountedInstances<T>::m_instanceCount = 0;

//=====================================================================================//
//                                 bool is_lowercase()                                 //
//=====================================================================================//
inline bool is_lowercase(const std::string &str)
{
	for(auto i = str.begin(); i != str.end(); ++i)
	{
		if(isalpha(*i) && !islower(*i)) return false;
	}
	return true;
}

#include "exception.h"
#include "safety.h"
#include "comptr.h"
#include "guid.h"
#include "module.h"
#include "CriticalSection.h"
#include "entry.h"
#include "Stream.h"
#include "WndHandle.h"

#endif // !defined(__PRECOMP_H_INCLUDED_5992692519605117__)