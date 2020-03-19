/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: ����������� ����, ��������� ���� �������� � �������
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   15.05.2000

************************************************************************/
#ifndef _V_FILE_H_
#define _V_FILE_H_

class VFile
{
public:
	//	���� ������
	enum {VFILE_EOF=0,		//	����� �� ������� ����� - m_iFilePos == m_iSize - ����������� ������� ������
		  VFILE_SOF=1,		//	������� ����� �� ������ ����� - m_iFilePos == 0
		  VFILE_NONE=2,		//	���� �� ��������
		  VFILE_OK=3};		//	��� � �������
		 
	//	���� ��� ����������������
	enum {VFILE_END=0,			//	�� ����� �����
		  VFILE_CUR=1,			//	�� ������� �������
		  VFILE_SET=2};		//	�� ������ �����
private:
	char *m_pFileName;	//	��� �����
	unsigned char *m_pData;		//	������, ��������� �� �����
	unsigned int m_iFileSize;		//	������ ������
	unsigned int m_iFilePos;	//	������� ���������
private:
	int m_iErrorCode;			//	��� ������
//************************ ������������ *****************************************//
public:
	VFile();
	VFile(const char *pFileName);
	VFile(FILE* hFile,const char *pFileName = 0);
	VFile(const unsigned char *pData,unsigned int iSize,const char *pFileName = 0);
	virtual ~VFile();
//************************ ���������� *****************************************//
public:
	bool Load(const char *pFileName);
	bool Load(FILE* hFile,const char *pFileName = 0);
	bool Load(const unsigned char *pData,unsigned int iSize,const char *pFileName = 0);
//************** ������� ����������� ������ � ������ **************************//
public:	//	��������, ����������� ������ � ������
	//	������ �� �����
	unsigned int Read(void *pBuff,unsigned int iSize);	// ������ � �����
	unsigned char ReadChar(void);	// ������ ������ �������
	//	���������������� � �����
	void Seek(int offset,int origin);	//	����������� ��������� � ����� �����
	void Rewind(void);	// ����������� ��������� � ������ �����
public:	//	�������������� �������
	inline const unsigned char* Data(void);
	inline unsigned int Size(void);
	inline int ErrorCode(void);
	inline const char* Name(void);
	inline unsigned int Tell(void);	//	������� ��������� ���������
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