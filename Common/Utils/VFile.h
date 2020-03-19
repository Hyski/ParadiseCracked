/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: виртуальный файл, поддержка всех опреаций с файлами
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   15.05.2000

************************************************************************/
#ifndef _V_FILE_H_
#define _V_FILE_H_

class VFile
{
public:
	//	коды ошибок
	enum {VFILE_EOF=0,		//	выход за пределы файла - m_iFilePos == m_iSize - предпринята попытка чтения
		  VFILE_SOF=1,		//	попытка выйти за начало файла - m_iFilePos == 0
		  VFILE_NONE=2,		//	файл не загружен
		  VFILE_OK=3};		//	все в порядке
		 
	//	коды для позиционирования
	enum {VFILE_END=0,			//	от конца файла
		  VFILE_CUR=1,			//	от текущей позиции
		  VFILE_SET=2};		//	от начала файла
private:
	char *m_pFileName;	//	имя файла
	unsigned char *m_pData;		//	данные, считанные из файла
	unsigned int m_iFileSize;		//	размер данных
	unsigned int m_iFilePos;	//	позиция указателя
private:
	int m_iErrorCode;			//	код ошибки
//************************ Конструкторы *****************************************//
public:
	VFile();
	VFile(const char *pFileName);
	VFile(FILE* hFile,const char *pFileName = 0);
	VFile(const unsigned char *pData,unsigned int iSize,const char *pFileName = 0);
	virtual ~VFile();
//************************ Загрузчики *****************************************//
public:
	bool Load(const char *pFileName);
	bool Load(FILE* hFile,const char *pFileName = 0);
	bool Load(const unsigned char *pData,unsigned int iSize,const char *pFileName = 0);
//************** Функции имитирующие работу с файлом **************************//
public:	//	операции, имитирующие работу с файлом
	//	чтение из файла
	unsigned int Read(void *pBuff,unsigned int iSize);	// чтение в буфер
	unsigned char ReadChar(void);	// чтение одного символа
	//	позиционирование в файле
	void Seek(int offset,int origin);	//	перемещение указателя в любое место
	void Rewind(void);	// перемещение указателя в начало файла
public:	//	информационные функции
	inline const unsigned char* Data(void);
	inline unsigned int Size(void);
	inline int ErrorCode(void);
	inline const char* Name(void);
	inline unsigned int Tell(void);	//	текущее положение указателя
	const char* Extension(void);
public:
	void Close(void);
private:
	void InZero(void);
};

const unsigned char* VFile::Data(void)
{
	return m_pData;
}

unsigned int VFile::Size(void)
{
	return m_iFileSize;
}

int VFile::ErrorCode(void)
{
	return m_iErrorCode;
}

const char* VFile::Name(void)
{
	return m_pFileName;
}

unsigned int VFile::Tell(void)
{
	return m_iFilePos;
}

#endif	//_V_FILE_H_