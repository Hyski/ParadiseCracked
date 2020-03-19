#pragma once

#include <string>
#include <cassert>
#include <typeinfo>
#include <windows.h>
#include <mll/debug/exception.h>

namespace mll
{
namespace utils
{

MLL_MAKE_EXCEPTION(cannot_create_file_mapping,mll::debug::exception);

//=====================================================================================//
//                                    class oneobj                                     //
//=====================================================================================//
template<typename T>
class oneobj
{
#pragma pack(push,1)
	struct InstanceInfo
	{
		volatile long m_refCount;
		T *m_instance;
	};
#pragma pack(pop)

	HANDLE m_fm;
	InstanceInfo *m_info;

public:
	oneobj()
	:	m_fm(0),
		m_info(0)
	{
		init(makeZone());
	}

	oneobj(const std::string &zone)
	:	m_fm(0),
		m_info(0)
	{
		const int pid = (int)GetCurrentProcessId();
		std::string zonename;
		zonename.reserve(zone.size() + sizeof(pid)*2);
		zonename.append(zone);
		zonename.resize(zone.size() + sizeof(pid)*2);
		encode(&zonename[zone.size()],(const char *)(&pid),sizeof(pid));
		init(zonename);
	}

	~oneobj()
	{
		InterlockedDecrement(&m_info->m_refCount);

		if(m_info->m_refCount == 0)
		{
			delete m_info->m_instance;
		}

		UnmapViewOfFile(m_info);
		CloseHandle(m_fm);
	}

	T *get() const { return m_info->m_instance; }

private:
	static void encode(char *dest, const char *src, size_t count)
	{
		static const char symtbl[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

		for(size_t i = 0; i != count; ++i)
		{
			*dest++ = symtbl[src[i]&0xF];
			*dest++ = symtbl[(src[i]>>4)&0xF];
		}
	}

	static std::string makeZone()
	{
		const int pid = (int)GetCurrentProcessId();
		const std::string temp = typeid(T).raw_name();
		std::string result;

		result.resize(temp.size()*2 + sizeof(pid)*2);

		encode(&result[0],&temp[0],temp.size());
		encode(&result[temp.size()*2],(const char *)(&pid),sizeof(pid));

		return result;
	}

	void init(const std::string &zone)
	{
		m_fm = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			sizeof(InstanceInfo),
			zone.c_str());

		if(m_fm == NULL)
		{
			throw cannot_create_file_mapping("CreateFileMapping failed. Possible cause: absence of pagefile");
		}

		const bool createObj = (GetLastError() != ERROR_ALREADY_EXISTS);

		m_info = (InstanceInfo *)MapViewOfFile(m_fm,FILE_MAP_WRITE,0,0,0);

		if(createObj)
		{
			m_info->m_instance = new T;
			m_info->m_refCount = 0;
		}

		InterlockedIncrement(&m_info->m_refCount);
	}
};

}
}


