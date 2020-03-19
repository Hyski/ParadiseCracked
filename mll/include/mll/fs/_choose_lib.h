#if !defined(___CHOOSE_LIB_H_INCLUDED_7552370787048739__) && !defined(__COMPILING_FS_LIBRARIES__)
#define ___CHOOSE_LIB_H_INCLUDED_7552370787048739__

#if defined(__ICL) && __ICL>=600
#	define FS_COMPILER_STAMP	"ic6_"
#elif defined(_MSC_VER)
#	if _MSC_VER<1300
#		define FS_COMPILER_STAMP	"msvc6_"
#	else
#		define FS_COMPILER_STAMP	"msvc7_"
#	endif
#else
#	error Unknown compiler
#endif

#define FS_BASE_NAME		"fs_"
#define FS_BASE_POSTFIX	".lib"

#if defined(_DEBUG)
#	define	FS_DEBUG_OR_RELEASE		"d"
#else
#	define	FS_DEBUG_OR_RELEASE		"r"
#endif

#if defined(FS_NO_USE_DLL)
#	define	FS_LINK_TYPE				"s"
#else
#	define	FS_LINK_TYPE				"d"
#endif

#if defined(_MT) && defined(_DLL)
#	define	FS_CRT_LIBRARY				"d"
#elif defined(_MT)
#	define	FS_CRT_LIBRARY				"m"
#else
#	define	FS_CRT_LIBRARY				"s"
#endif

#define	FS_LIBRARY		FS_COMPILER_STAMP FS_BASE_NAME FS_DEBUG_OR_RELEASE	\
						FS_LINK_TYPE FS_CRT_LIBRARY FS_BASE_POSTFIX

//#pragma comment(lib,FS_LIBRARY)

#if defined(__TELL_FS_USED_LIBRARY__)
#pragma message("Using library " FS_LIBRARY)
#endif

#undef FS_COMPILER_STAMP
#undef FS_LINK_TYPE
#undef FS_CRT_LIBRARY
#undef FS_LIBRARY
#undef FS_BASE_NAME
#undef FS_BASE_POSTFIX
#undef FS_DEBUG_OR_RELEASE

#endif // !defined(___CHOOSE_LIB_H_INCLUDED_7552370787048739__)