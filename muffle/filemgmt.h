#if !defined(__FILEMGMT_H_INCLUDED_7734585223535450__)
#define __FILEMGMT_H_INCLUDED_7734585223535450__

#include "NotifyReceiver.h"
#include <hash_map>

class RealFile;

//=====================================================================================//
//                                   class FileMgmt                                    //
//=====================================================================================//
class FileMgmt
{
	typedef std::hash_map<std::string,RealFile *> FileMap_t;
	NotifyThread *m_nthread;
	FileMap_t m_fileMap;
	ci_VFileSystem *m_vfs;

	CriticalSection m_mapGuard;
	HANDLE m_htimer;

	typedef NotifyAdaptor<FileMgmt> Adaptor_t;
	Adaptor_t m_timerAdaptor;

	void setTimer();
	void onTimer();

	unsigned m_totalSize;

public:
	FileMgmt(NotifyThread *, ci_VFileSystem *);
	~FileMgmt();

	Stream createFile(const std::string &);

	unsigned getTotalSize();
	unsigned getFileCount();

	void outputDebugInfo(SndServices*);
};

#endif // !defined(__FILEMGMT_H_INCLUDED_7734585223535450__)