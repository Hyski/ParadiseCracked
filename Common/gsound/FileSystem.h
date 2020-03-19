#if !defined(__VIRTUAL_FILE_SYSTEM_WRAPPER_INCLUDED__)
#define __VIRTUAL_FILE_SYSTEM_WRAPPER_INCLUDED__

#include "../../sound/VFS.h"
#include "../DataMgr/DataMgr.h"
#include <string>
#include <vector>
#include <map>

class Dir;

//=====================================================================================//
//                                   class cc_VDirIt                                   //
//=====================================================================================//
class cc_VDirIt : public ci_VDirIt
{
	typedef std::vector<DataMgr::FileInfo> files_t;
	files_t m_Files;
	int m_Current;
	std::string m_Mask;
	std::string m_Path;

	void findFirst();

public:
	cc_VDirIt(const char *mask);
	~cc_VDirIt();

	bool isDone() const; 
	void next();
	ci_VDirIt *clone() const;

	bool isArchive() const;
	bool isHidden() const;
	bool isReadOnly() const;
	bool isDir() const;
	bool isSystem() const;

	const std::string &getFullName() const;
	const std::string &getName() const;
	const std::string &getExtension() const;

	void Release();
};

//=====================================================================================//
//                                   class cc_VFile                                    //
//=====================================================================================//
class cc_VFile : public ci_VFile
{
	VFile *m_File;

public:
	cc_VFile(const char *name);
	~cc_VFile();

	int    Read(void *Buffer,const unsigned int& Size, const unsigned int& Items) const;
	int    Feof() const;
	int    Size() const;
	long   Seek(long offset, int from) const;
	long   Tell(void) const;
	int    Flush() const;
	void   Rewind(void) const;
	std::string GetPath() const;

	const void *getData() const;

	void   Release();
};

//=====================================================================================//
//                                class cc_VFileSystem                                 //
//=====================================================================================//
class cc_VFileSystem : public ci_VFileSystem
{
	static cc_VFileSystem *m_Instance;

	typedef std::map<std::string,unsigned> RefCounters_t;
	mutable RefCounters_t m_refCounters;

public:
	ci_VFile* CreatFile(const char* FName, const unsigned int& Mode) const;
	ci_VDirIt *GetDir(const char* Path);

	bool releaseFile(const char *);

	static void initInstance();
	static void shutInstance();
	static ci_VFileSystem *instance();
};

#endif