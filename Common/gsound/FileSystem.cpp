#include "precomp.h"
#include "FileSystem.h"
#include "../Utils/Dir.h"
#include <shlwapi.h> // Для масок

#include <sstream>
//#include <fstream>

cc_VFileSystem *cc_VFileSystem::m_Instance = 0;

//=====================================================================================//
//                               cc_VDirIt::cc_VDirIt()                                //
//=====================================================================================//
cc_VDirIt::cc_VDirIt(const char *mask)
:	m_Current(0),
	m_Mask(mask)
{
	char buffer[_MAX_PATH];
	strcpy(buffer,mask);

	// Это работает только не с текущим каталогом
	// т. е. нельзя указать просто *.*
	// Текущий каталог следует указывать явно .\*.*
	PathRemoveBlanks(buffer);
	PathRemoveFileSpec(buffer);
	PathAddBackslash(buffer);

	std::replace(buffer,buffer+_MAX_PATH,'\\','/');

	m_Path = buffer;

	DataMgr::Scan(m_Path.c_str(), m_Files);

	for (int i = 0; i < m_Files.size(); i++)
	{
		m_Files[i].m_FullName = m_Path+m_Files[i].m_FullName;
	}

	findFirst();
}

//=====================================================================================//
//                               cc_VDirIt::~cc_VDirIt()                               //
//=====================================================================================//
cc_VDirIt::~cc_VDirIt()
{
}

//=====================================================================================//
//                                 cc_VDirIt::isDone()                                 //
//=====================================================================================//
bool cc_VDirIt::isDone() const
{
	if (m_Current >= m_Files.size()) return true;
	return false;
}

//=====================================================================================//
//                               cc_VDirIt::findFirst()                                //
//=====================================================================================//
void cc_VDirIt::findFirst()
{
	for (; (m_Current<m_Files.size()) && 
		m_Files[m_Current].m_Extension != ".seng";
//		(!PathMatchSpec((m_Path+m_Files[m_Current].m_FullName).c_str(), m_Mask.c_str())); 
		m_Current++);
}

//=====================================================================================//
//                                  cc_VDirIt::next()                                  //
//=====================================================================================//
void cc_VDirIt::next()
{
	do
		++m_Current;
	while ((m_Current<m_Files.size()) && 
		m_Files[m_Current].m_Extension != ".seng");
		//(!PathMatchSpec((m_Path+m_Files[m_Current].m_FullName).c_str(), m_Mask.c_str())));
}

//=====================================================================================//
//                                 cc_VDirIt::clone()                                  //
//=====================================================================================//
ci_VDirIt *cc_VDirIt::clone() const
{
	return new cc_VDirIt(m_Mask.c_str());
}

//=====================================================================================//
//                               cc_VDirIt::isArchive ()                               //
//=====================================================================================//
bool cc_VDirIt::isArchive () const
{
	return (m_Files[m_Current].m_Attr&DataMgr::FileInfo::ARCHIVE) != 0;
}

//=====================================================================================//
//                               cc_VDirIt::isHidden ()                                //
//=====================================================================================//
bool cc_VDirIt::isHidden () const
{
	return (m_Files[m_Current].m_Attr&DataMgr::FileInfo::HIDDEN) != 0;
}

//=====================================================================================//
//                              cc_VDirIt::isReadOnly ()                               //
//=====================================================================================//
bool cc_VDirIt::isReadOnly () const
{
	return (m_Files[m_Current].m_Attr&DataMgr::FileInfo::READONLY) != 0;
}

//=====================================================================================//
//                                 cc_VDirIt::isDir ()                                 //
//=====================================================================================//
bool cc_VDirIt::isDir () const
{
	return (m_Files[m_Current].m_Attr&DataMgr::FileInfo::SUBDIR) != 0;
}

//=====================================================================================//
//                               cc_VDirIt::isSystem ()                                //
//=====================================================================================//
bool cc_VDirIt::isSystem () const
{
	return (m_Files[m_Current].m_Attr&DataMgr::FileInfo::SYSTEM) != 0;
}

//=====================================================================================//
//                              cc_VDirIt::getFullName()                               //
//=====================================================================================//
const std::string &cc_VDirIt::getFullName() const
{
	return m_Files[m_Current].m_FullName;
}

//=====================================================================================//
//                              cc_VDirIt::getExtension()                              //
//=====================================================================================//
const std::string &cc_VDirIt::getExtension() const
{
	return m_Files[m_Current].m_Extension;
}

//=====================================================================================//
//                                cc_VDirIt::getName()                                 //
//=====================================================================================//
const std::string &cc_VDirIt::getName() const
{
	return m_Files[m_Current].m_Name;
}

//=====================================================================================//
//                                cc_VDirIt::Release()                                 //
//=====================================================================================//
void cc_VDirIt::Release()
{
	delete this;
}

//=====================================================================================//
//                                cc_VFile::cc_VFile()                                 //
//=====================================================================================//
cc_VFile::cc_VFile(const char *name)
{
	m_File = DataMgr::Load(name);
	if(m_File->Size() == 0)
	{
		DataMgr::Release(name);
		m_File = 0;
	}
}

//=====================================================================================//
//                                cc_VFile::~cc_VFile()                                //
//=====================================================================================//
cc_VFile::~cc_VFile()
{
	if(m_File)
	{
		DataMgr::Release(m_File->Name());
	}
}

//=====================================================================================//
//                             int cc_VFile::Read() const                              //
//=====================================================================================//
int cc_VFile::Read(void *buffer, const unsigned &size, const unsigned &count) const
{
	return m_File->Read(buffer,size*count);
}

//=====================================================================================//
//                             int cc_VFile::Feof() const                              //
//=====================================================================================//
int cc_VFile::Feof() const
{
	return m_File->Tell() >= m_File->Size();
}

//=====================================================================================//
//                             int cc_VFile::Size() const                              //
//=====================================================================================//
int cc_VFile::Size() const
{
	if(m_File) return m_File->Size();
	return 0;
}

//=====================================================================================//
//                             long cc_VFile::Seek() const                             //
//=====================================================================================//
long cc_VFile::Seek(long offset, int origin) const
{
	int org = VFile::VFILE_CUR;

	switch(origin)
	{
		case CURRENT:
			org = VFile::VFILE_CUR;
			break;
		case END:
			org = VFile::VFILE_END;
			break;
		case BEGIN:
			org = VFile::VFILE_SET;
			break;
	}

	m_File->Seek(offset,org);

	return m_File->Tell();
}

//=====================================================================================//
//                             long cc_VFile::Tell() const                             //
//=====================================================================================//
long cc_VFile::Tell() const
{
	return m_File->Tell();
}

//=====================================================================================//
//                            void cc_VFile::Rewind() const                            //
//=====================================================================================//
void cc_VFile::Rewind() const
{
	m_File->Seek(0,VFile::VFILE_SET);
}

//=====================================================================================//
//                              void cc_VFile::Release()                               //
//=====================================================================================//
void cc_VFile::Release()
{
	delete this;
}

//=====================================================================================//
//                        const void *cc_VFile::getData() const                        //
//=====================================================================================//
const void *cc_VFile::getData() const
{
	return m_File->Data();
}

//=====================================================================================//
//                     ci_VFile *cc_VFileSystem::CreatFile() const                     //
//=====================================================================================//
ci_VFile *cc_VFileSystem::CreatFile(const char *name, const unsigned &/*mode*/) const
{
	return new cc_VFile(name);
}

//=====================================================================================//
//                         ci_VDirIt *cc_VFileSystem::GetDir()                         //
//=====================================================================================//
ci_VDirIt *cc_VFileSystem::GetDir(const char *dir)
{
	return new cc_VDirIt(dir);
}

//=====================================================================================//
//                         void cc_VFileSystem::initInstance()                         //
//=====================================================================================//
void cc_VFileSystem::initInstance()
{
	if (!m_Instance) m_Instance = new cc_VFileSystem;
}

//=====================================================================================//
//                         void cc_VFileSystem::shutInstance()                         //
//=====================================================================================//
void cc_VFileSystem::shutInstance()
{
	if (m_Instance)
	{
//		static std::ofstream fl("ofidump.log");
//		for(RefCounters_t::iterator itor = m_Instance->m_refCounters.begin(); itor != m_Instance->m_refCounters.end(); ++itor)
//		{
//			fl << itor->first << " = " << itor->second << std::endl;
//		}
		delete m_Instance;
		m_Instance = 0;
	}
}

//=====================================================================================//
//                     ci_VFileSystem *cc_VFileSystem::instance()                      //
//=====================================================================================//
ci_VFileSystem *cc_VFileSystem::instance()
{
	return m_Instance;
}

//=====================================================================================//
//                         bool cc_VFileSystem::releaseFile()                          //
//=====================================================================================//
//bool cc_VFileSystem::releaseFile(const char *name)
//{
//	m_refCounters
//	return !m_refCounters[name];
//}