#if !defined(___EXPORT_RULES_H_INCLUDED_7552370787048739__)
#define ___EXPORT_RULES_H_INCLUDED_7552370787048739__


/*
#if defined(_DEBUG)
#	define	FS_DEBUG_OR_RELEASE		"_dbg"
#else
#	define	FS_DEBUG_OR_RELEASE		""
#endif

#define FS_LIBRARY "filesystem" FS_DEBUG_OR_RELEASE ".lib"
*/


#if defined(FS_USE_DLL)
#	if defined(__COMPILING_FS_LIBRARIES__)
#		define FILE_SYSTEM_EXPORT __declspec(dllexport)
#	else
#		define FILE_SYSTEM_EXPORT __declspec(dllimport)
//#pragma comment (lib, FS_LIBRARY)
#	endif
#else
#	define FILE_SYSTEM_EXPORT
#endif



/*
#if defined(_DEBUG)
#	define	HDD_SUB_DEBUG_OR_RELEASE		"_dbg"
#else
#	define	HDD_SUB_DEBUG_OR_RELEASE		""
#endif

#define HDD_SUB_LIBRARY "hdd_sub_fs" HDD_SUB_DEBUG_OR_RELEASE ".lib"
*/


//#if defined (FS_USE_DLL)
//#define HDD_SUBSYSTEM_EXPORT __declspec(dllexport)
//#else
//#define HDD_SUBSYSTEM_EXPORT __declspec(dllimport)
//#pragma comment (lib, HDD_SUB_LIBRARY)
//#endif



/*
#if defined(_DEBUG)
#	define	ZIP_SUB_DEBUG_OR_RELEASE		"_dbg"
#else
#	define	ZIP_SUB_DEBUG_OR_RELEASE		""
#endif

#define ZIP_SUB_LIBRARY "zip_sub_fs" ZIP_SUB_DEBUG_OR_RELEASE ".lib"
*/


//#if defined (FS_USE_DLL)
//#define ZIP_SUBSYSTEM_EXPORT __declspec(dllexport)
//#else
//#define ZIP_SUBSYSTEM_EXPORT __declspec(dllimport)
//#pragma comment (lib, ZIP_SUB_LIBRARY)
//#endif



#endif // !defined(___EXPORT_RULES_H_INCLUDED_7552370787048739__)
