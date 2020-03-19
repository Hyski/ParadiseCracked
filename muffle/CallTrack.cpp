#include "precomp.h"
#include "CallTrack.h"

//=====================================================================================//
//                               void CallTrack::enter()                               //
//=====================================================================================//
void CallTrack::enter(const char *func)
{
	HANDLE thread = GetCurrentThread();
	
	thread_map_t::iterator i;

	{
		Entry guard(m_mapGuard);
		i = m_threadMap.find(thread);
		if(i == m_threadMap.end())
		{
			i = m_threadMap.insert(std::make_pair(thread,callstack_t())).first;
		}
	}

	i->second.push_back(func);
}

//=====================================================================================//
//                               void CallTrack::leave()                               //
//=====================================================================================//
void CallTrack::leave()
{
	HANDLE thread = GetCurrentThread();
	
	thread_map_t::iterator i;

	{
		Entry guard(m_mapGuard);
		i = m_threadMap.find(thread);
	}

	i->second.pop_back();

	if(i->second.empty())
	{
		Entry guard(m_mapGuard);
		m_threadMap.erase(i);
	}
}

//=====================================================================================//
//                        CallTrack &CallTrack::privInstance()                         //
//=====================================================================================//
CallTrack &CallTrack::privInstance()
{
	static CallTrack track;
	return track;
}