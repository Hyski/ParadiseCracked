/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   10.09.2000

************************************************************************/
#ifndef _TBL_FILE_H_
#define _TBL_FILE_H_

class TblFile
{
private:
	std::vector<std::vector<std::string> > m_vData;
private:
	int m_iSizeX;
	int m_iSizeY;
public:
	TblFile();
	TblFile(VFile *pVFile);
	virtual ~TblFile();
public:
	bool Load(VFile *pVFile);
public:
	//	i - номер строки, j - номер столбца
	void GetCell(unsigned int i,unsigned int j,std::string& cell);
	const std::string& operator()(unsigned int i,unsigned int j);
	//	------------------------------------------------------------
	bool Find(const char *pFind,unsigned int *i,unsigned int *j);
	bool FindInCol(const char *pFind,unsigned int *i,unsigned int j);
	bool FindInRow(const char *pFind,unsigned int i,unsigned int *j);
public:
	int SizeX(void);
	int SizeY(void);
};

inline int TblFile::SizeX(void)
{
	return m_iSizeX;
}

inline int TblFile::SizeY(void)
{
	return m_iSizeY;
}

#endif	//_TBL_FILE_H_