#if !defined(__DM8ERROR_TRANSLATOR_INCLUDED__)
#define __DM8ERROR_TRANSLATOR_INCLUDED__


#if defined(_DEBUG) && defined(TURN_ON_ALL_DEBUG_STUFF)
#	define SAFE_CALL(x)	if (FAILED(x)) throw SND_EXCEPTION(std::string(__strPFuncName)+" - Function call failed:\n"+#x+"\n"+cc_DM8Error::getDescription(x))
#else
#	define SAFE_CALL(x)	x
#endif

#include <dmusici.h>

/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// class cc_SoundError //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class cc_SoundError : public cc_Unexpected
{
public:
	cc_SoundError(HRESULT,const char *);
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// class cc_DM8Error ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class cc_DM8Error
{
public:
	static const char *getDescription(HRESULT);
	static void safe_call(HRESULT, const char *);
	static std::string getMusicTime();
	static std::string unsigned2String(unsigned);
	static std::string hexstr(const void *);
};

#endif