#include "precomp.h"
#include "filemgmt.h"
#include "RealFile.h"
#include "NotifyThread.h"

//=====================================================================================//
//                                FileMgmt::FileMgmt()                                 //
//=====================================================================================//
FileMgmt::FileMgmt(NotifyThread *nthread, ci_VFileSystem *vfs)
:	m_vfs(vfs),
	m_timerAdaptor(this,&FileMgmt::onTimer),
	m_nthread(nthread),
	m_totalSize(0)
{
	m_htimer = CreateWaitableTimer(NULL,FALSE,NULL);
	setTimer();
	m_nthread->addNotify(m_htimer,&m_timerAdaptor);
}

//=====================================================================================//
//                                FileMgmt::~FileMgmt()                                //
//=====================================================================================//
FileMgmt::~FileMgmt()
{
	m_nthread->removeNotify(m_htimer);
	CloseHandle(m_htimer);

	{
		Entry guard(m_mapGuard);

		for(FileMap_t::iterator i = m_fileMap.begin(); i != m_fileMap.end();)
		{
			m_totalSize -= i->second->getLength();
			i->second->release();
			m_fileMap.erase(i++);
		}
	}
}

//=====================================================================================//
//                            Stream FileMgmt::createFile()                            //
//=====================================================================================//
Stream FileMgmt::createFile(const std::string &bad_name)
{
	std::string tname(bad_name);
	std::transform(tname.begin(),tname.end(),tname.begin(),tolower);
	std::replace(tname.begin(),tname.end(),'\\','/');

	Entry guard(m_mapGuard);
	FileMap_t::iterator i = m_fileMap.find(tname);

	if(i == m_fileMap.end())
	{
		static unsigned counter = 0;
		RealFile *file = new RealFile(m_vfs,tname,counter++);
		file->addRef();
		m_fileMap.insert(std::make_pair(tname,file));
		m_totalSize += file->getLength();
		return Stream(file);
	}
	else
	{
		return Stream(i->second);
	}
}

//=====================================================================================//
//                              void FileMgmt::setTimer()                              //
//=====================================================================================//
void FileMgmt::setTimer()
{
	LARGE_INTEGER period;
	period.QuadPart = -10;
	period.QuadPart *= 10000000;	// количество единиц времени в секунде
	if(!SetWaitableTimer(m_htimer,&period,0,NULL,NULL,FALSE))
	{
		throw sound_error("Cannot set waitable timer");
	}
}

//=====================================================================================//
//                              void FileMgmt::onTimer()                               //
//=====================================================================================//
void FileMgmt::onTimer()
{
	Entry guard(m_mapGuard);
	unsigned time = timeGetTime();

	for(FileMap_t::iterator i = m_fileMap.begin(); i != m_fileMap.end();)
	{
		if(i->second->getRefCounter() == 1 &&
			(time - i->second->getLastUseTime()) > 10000)
		{
			m_totalSize -= i->second->getLength();
			i->second->release();
			m_fileMap.erase(i++);
		}
		else
		{
			++i;
		}
	}

	setTimer();
}

//=====================================================================================//
//                          unsigned FileMgmt::getTotalSize()                          //
//=====================================================================================//
unsigned FileMgmt::getTotalSize()
{
	return m_totalSize;
}

//=====================================================================================//
//                          unsigned FileMgmt::getFileCount()                          //
//=====================================================================================//
unsigned FileMgmt::getFileCount()
{
	return m_fileMap.size();
}

//=====================================================================================//
//                          void FileMgmt::outputDebugInfo()                           //
//=====================================================================================//
void FileMgmt::outputDebugInfo(SndServices *svc)
{
	Entry guard(m_mapGuard);

	unsigned y = 0;
	for(FileMap_t::iterator i = m_fileMap.begin(); i != m_fileMap.end(); ++i)
	{
		svc->dbg_printf(700,500+y,i->first.c_str());
		y += 20;
	}
}