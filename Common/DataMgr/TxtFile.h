/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   04.06.2000

************************************************************************/
#ifndef _TXT_FILE_H_
#define _TXT_FILE_H_

#include "../Utils/VFile.h"

//#define CASUS(msg) mll::debug::exception(msg)

class TxtFile
{
private:
	std::vector<std::vector<std::string> > m_vData;
	std::string m_FileName;		
public:
	TxtFile();
	TxtFile(VFile *pVFile);
	virtual ~TxtFile();
public:
	bool Load(VFile *pVFile);
public:
	inline int SizeX(unsigned int i);
	inline int SizeY(void);
	//	i - номер строки, j - номер столбца
	void GetCell(unsigned int i,unsigned int j,std::string& cell);
	const std::string& operator()(unsigned int i,unsigned int j);
	bool Find(const char *pFind,std::string& cell);
	bool Find(const char *pFind,unsigned int *i,unsigned int *j);
	bool FindInCol(const char *pFind,unsigned int *i,unsigned int j);
	bool FindInRow(const char *pFind,unsigned int i,unsigned int *j);
private:
	bool GetCell(VFile *pVFile,std::string& str,bool *pbEOS);

};

int TxtFile::SizeY(void)
{
	return m_vData.size();
}

int TxtFile::SizeX(unsigned int i)
{
	if(i<m_vData.size())
		return m_vData[i].size();
	else
		throw CASUS(m_FileName+std::string(": выход за границу массива.\n"));
}

#endif	//_TXT_FILE_H_