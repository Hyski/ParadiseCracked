#if !defined(__COMPILER_MACROS_H_INCLUDED_7552370787048739__)
#define __COMPILER_MACROS_H_INCLUDED_7552370787048739__

#if defined(_DEBUG)
#	define	MLL_DEBUG_SUFFIX	"_dbg"
#else
#	define	MLL_DEBUG_SUFFIX	"_rls"
#endif

#if defined(_DLL)
#	define	MLL_DYNAMIC_SUFFIX	"_dyn"
#else
#	define	MLL_DYNAMIC_SUFFIX	"_stc"
#endif

#if defined(__ICL)
#	define	MLL_COMPILER_SUFFIX	"_icl"
#else
#	if _MSC_VER>=1300
#		define	MLL_COMPILER_SUFFIX	"_vc7"
#	elif _MSC_VER>=1200
#		define	MLL_COMPILER_SUFFIX	"_vc6"
#	endif
#endif

#define MLL_COMPOSE_LIBRARY_CUSTOM(BaseName,Compiler,Debug,Dynamic)	\
	BaseName Compiler Dynamic Debug ".lib"

#define MLL_COMPOSE_LIBRARY(BaseName)	\
	MLL_COMPOSE_LIBRARY_CUSTOM(BaseName,MLL_COMPILER_SUFFIX,MLL_DEBUG_SUFFIX,MLL_DYNAMIC_SUFFIX)

#endif // !defined(__COMPILER_MACROS_H_INCLUDED_7552370787048739__)