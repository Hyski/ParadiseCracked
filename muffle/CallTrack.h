#if !defined(__CALL_TRACK_H_INCLUDED_7552370787048739__)
#define __CALL_TRACK_H_INCLUDED_7552370787048739__

#include <map>

//=====================================================================================//
//                                   class CallTrack                                   //
//=====================================================================================//
class CallTrack
{
	typedef std::vector<const char *> callstack_t;
	typedef std::map<HANDLE,callstack_t> thread_map_t;

	thread_map_t m_threadMap;

	CriticalSection m_mapGuard;
	static CallTrack &privInstance();

	friend class FuncEntry;

	void enter(const char *);
	void leave();

	static CallTrack &instance() { return privInstance(); }

public:
	std::string getInfo();
};

//=====================================================================================//
//                                   class FuncEntry                                   //
//=====================================================================================//
class FuncEntry
{
public:
	FuncEntry(const char *name) { CallTrack::instance().enter(name); }
	~FuncEntry() { CallTrack::instance().leave(); }
};

#if defined(_DEBUG)
#define FENTER(N)	FuncEntry __fentry_no_same_name_in_func(N)
#else
#define FENTER(N)
#endif	

#endif // !defined(__CALL_TRACK_H_INCLUDED_7552370787048739__)