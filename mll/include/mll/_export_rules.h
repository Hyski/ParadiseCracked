#if !defined(__EXPORT_RULES_ALREADY_INCLUDED__)
#define __EXPORT_RULES_ALREADY_INCLUDED__

#if defined(MLL_USE_DLL)
#	if defined(__COMPILING_MLL_LIBRARIES__)
#		define	MLL_EXPORT			__declspec(dllexport)
#	else
#		define	MLL_EXPORT			__declspec(dllimport)
#	endif
#else
#	define MLL_EXPORT
#endif

#endif
