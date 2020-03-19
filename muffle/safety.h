#if !defined(__SAFETY_H_INCLUDED_2903341426519653__)
#define __SAFETY_H_INCLUDED_2903341426519653__

#if defined(_DEBUG)
#define SAFE_CALL(C)	_do_call((C),#C)
#else
#define SAFE_CALL(C)	(C)
#endif

const char *get_error_text(HRESULT);
HRESULT _do_call(HRESULT, const char *);

//=====================================================================================//
//                                  class noncopyable                                  //
//=====================================================================================//
class noncopyable
{
	noncopyable(const noncopyable &);
	noncopyable &operator=(const noncopyable &);
public:
	noncopyable() {}
};

#endif // !defined(__SAFETY_H_INCLUDED_2903341426519653__)