/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   10.09.2000

************************************************************************/
#include "Precomp.h"
#include "TblFile.h"

//---------- Ћог файл ------------
#ifdef _DEBUG_SHELL
CLog _table;
#define table _table["table.log"]
#else
#define table /##/
#endif
//--------------------------------

TblFile::TblFile()
{
}

TblFile::TblFile(VFile *pVFile)
{
	Load(pVFile);
}

TblFile::~TblFile()
{
}

bool TblFile::Load(VFile *pVFile)
{
	std::vector<std::string> vStr;
	int len,max_len = 0;
	char *pBuff = 0;

	table("\n--- Load: <%s> ---\n",pVFile->Name());
	if(pVFile->Size())
	{
		m_vData.clear();
		pVFile->Rewind();
		//	считываем ширину и высоту таблицы
		pVFile->Read(&m_iSizeX,sizeof(int));
		pVFile->Read(&m_iSizeY,sizeof(int));
		table("    ...size: [%d:%d]\n",m_iSizeX,m_iSizeY);
		for(int i=0;i<m_iSizeY;i++)
		{
			vStr.clear();
			table(">>>>>>>>>>>>>>>>> string:\n");
			for(int j=0;j<m_iSizeX;j++)
			{
				pVFile->Read(&len,sizeof(int));
				if(len > max_len)
				{//	буфер слишком мал
					if(pBuff)
						delete[] pBuff;
					max_len = len;
					pBuff = new char[max_len+1];
				}
				table("- ");
				if(len > 0)
				{
					pVFile->Read(pBuff,len);
					pBuff[len] = 0;
					table("[%s]",pBuff);
					vStr.push_back(pBuff);
				}
				else
				{
					vStr.push_back("");
				}
				table("\n");
				if((pVFile->ErrorCode() != VFile::VFILE_OK))
					throw CASUS("Ќеправильный формат файла.");
			}
			m_vData.push_back(vStr);
		}
		if(pBuff)
			delete[] pBuff;

		return true;
	}

	return false;
}

void TblFile::GetCell(unsigned int i,unsigned int j,std::string& cell)
{
	if(i>=m_vData.size())
		throw CASUS("TxtFile: выход за границу массива строк. \n");
	if(j>=m_vData[i].size())
		throw CASUS("TxtFile: выход за границу массива. \n");
	cell = m_vData[i][j];
}

const std::string& TblFile::operator()(unsigned int i,unsigned int j)
{
	if(i>=m_vData.size())
		throw CASUS("TxtFile: выход за границу массива строк. \n");
	if(j>=m_vData[i].size())
		throw CASUS("TxtFile: выход за границу массива. \n");
	return m_vData[i][j];
}

bool TblFile::Find(const char *pFind,unsigned int *i,unsigned int *j)
{
	if(pFind)
	{
		for(int ic=0;ic<m_vData.size();ic++)
		{
			for(int jc=0;jc<m_vData[ic].size();jc++)
			{
				if(strcmp(m_vData[ic][jc].c_str(),pFind)==0)
				{
					*i = ic;
					*j = jc;
					return true;
				}
			}
		}
	}

	return false;
}

bool TblFile::FindInCol(const char *,unsigned int *,unsigned int )
{
	return false;
}

bool TblFile::FindInRow(const char *,unsigned int ,unsigned int *)
{
	return false;
}
