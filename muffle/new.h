#if !defined(__NEW_H_INCLUDED_7552370787048739__)
#define __NEW_H_INCLUDED_7552370787048739__

//=====================================================================================//
//                                void *operator new()                                 //
//=====================================================================================//
inline void *operator new(size_t size, const char *file, unsigned line)
{
	void *ptr = ::operator new(size);
	std::ostringstream sstr;
	sstr << "Allocated " << ptr << "(" << size << ") at " << file << "(" << line << ")\n";
	OutputDebugString(sstr.str().c_str());
	return ptr;
}

//=====================================================================================//
//                               void operator delete()                                //
//=====================================================================================//
inline void operator delete(void *ptr, const char *file, unsigned line)
{
	::operator delete(ptr);
	std::ostringstream sstr;
	sstr << "Freed " << ptr << file << "(" << line << ")\n";
	OutputDebugString(sstr.str().c_str());

}

#define DEBUG_NEW	new(__FILE__,__LINE__)
#define	new			DEBUG_NEW

#endif // !defined(__NEW_H_INCLUDED_7552370787048739__)