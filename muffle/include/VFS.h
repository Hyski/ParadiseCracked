#if !defined(__VIRTUAL_FILE_SYSTEM_INCLUDED__)
#define __VIRTUAL_FILE_SYSTEM_INCLUDED__

#if defined(_UNDER_CARCASS_)
#include <interfaces/IFile.h>
#include <interfaces/IFileSystem.h>
typedef ci_File ci_VFile;
typedef ci_FileSystem ci_VFileSystem;
typedef ci_DirIt ci_VDirIt;
#else

//=====================================================================================//
//                                   class ci_VDirIt                                   //
//=====================================================================================//
class ci_VDirIt
{
public:
	virtual bool isDone() const = 0; 
	virtual void next() = 0;
	virtual ci_VDirIt *clone() const = 0;

	virtual bool isArchive() const = 0;
	virtual bool isHidden() const = 0;
	virtual bool isReadOnly() const = 0;
	virtual bool isDir() const = 0;
	virtual bool isSystem() const = 0;

	virtual const std::string &getFullName() const = 0;
	virtual const std::string &getName() const = 0;
	virtual const std::string &getExtension() const = 0;

	virtual void Release() = 0;
};

//=====================================================================================//
//                                   class ci_VFile                                    //
//=====================================================================================//
class ci_VFile
{
public:
	enum
	{
		CURRENT = 1,
		END     = 2,
		BEGIN   = 0
	};

public:
	virtual int    Read(void *Buffer,const unsigned int& Size, const unsigned int& Items) const=0 ; // Чтение из файла в Buffer Items элементов размера Size
	virtual int    Feof() const                                     =0 ;
	virtual int    Size() const                                     =0 ; // Возвращает размер файла
	virtual long   Seek(long offset, int from) const                =0 ; // Перемещение указателя на offset относительно from
	virtual long   Tell(void) const																	=0 ; // ф-я возвр. текущее положение указателя
	virtual void   Rewind(void) const                               =0 ; // перемещение указателя в начало файла

	virtual void   Release()										=0 ;

	virtual const void *getData() const								=0 ;
};

//=====================================================================================//
//                                class ci_VFileSystem                                 //
//=====================================================================================//
class ci_VFileSystem
{
public:
	enum
	{
		READ      = 0x01,
		WRITE     = 0x02,
		CREATENEW = 0x04,
		APPEND    = 0x08
	};

public:
	virtual ci_VFile* CreatFile(const char* FName, const unsigned int& Mode) const = 0; // Открыть(создать) файл
	virtual ci_VDirIt *GetDir(const char* Path)=0; // Возвращает вектор с FileInfo из файлов находящихся в Dir
};

#endif // defined(_UNDER_CARCASS_)

#endif