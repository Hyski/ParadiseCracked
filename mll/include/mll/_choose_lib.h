#if !defined(__MLL_LIBRARY_HAVE_BEEN_CHOSEN__) && !defined(__COMPILING_MLL_LIBRARIES__)
#define __MLL_LIBRARY_HAVE_BEEN_CHOSEN__

#if defined(__ICL) && __ICL>=600
#	define MLL_COMPILER_STAMP	"ic6_"
#elif defined(_MSC_VER)
#	if _MSC_VER<1300
#		define MLL_COMPILER_STAMP	"msvc6_"
#	else
#		define MLL_COMPILER_STAMP	"msvc7_"
#	endif
#else
#	error Unknown compiler
#endif

#define MLL_BASE_NAME		"mll_"
#define MLL_BASE_POSTFIX	".lib"

#if defined(_DEBUG)
#	define	MLL_DEBUG_OR_RELEASE		"d"
#else
#	define	MLL_DEBUG_OR_RELEASE		"r"
#endif

#if defined(MLL_USE_DLL)
#	define	MLL_LINK_TYPE				"d"
#else
#	define	MLL_LINK_TYPE				"s"
#endif

#if defined(_MT) && defined(_DLL)
#	define	MLL_CRT_LIBRARY				"d"
#elif defined(_MT)
#	define	MLL_CRT_LIBRARY				"m"
#else
#	define	MLL_CRT_LIBRARY				"s"
#endif

#if defined(_STLP_USE_STATIC_LIB)
#	define	MLL_STLPORT_LINKAGE			"s"
#else
#	define	MLL_STLPORT_LINKAGE			"d"
#endif

#define	MLL_LIBRARY		MLL_COMPILER_STAMP MLL_BASE_NAME MLL_DEBUG_OR_RELEASE	\
						MLL_LINK_TYPE MLL_CRT_LIBRARY MLL_STLPORT_LINKAGE MLL_BASE_POSTFIX

//#pragma comment(lib,MLL_LIBRARY)

#if defined(__TELL_MLL_USED_LIBRARY__)
#pragma message("Using library " MLL_LIBRARY)
#endif

#undef MLL_COMPILER_STAMP
#undef MLL_LINK_TYPE
#undef MLL_CRT_LIBRARY
#undef MLL_LIBRARY
#undef MLL_BASE_NAME
#undef MLL_BASE_POSTFIX
#undef MLL_DEBUG_OR_RELEASE
#undef MLL_STLPORT_LINKAGE

#endif
