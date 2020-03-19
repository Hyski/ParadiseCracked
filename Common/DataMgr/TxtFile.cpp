/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   04.06.2000

************************************************************************/
#include "Precomp.h"
#include "TxtFile.h"

//---------- Ћог файл ------------
#ifdef _NO_DEBUG_SHELL
CLog tfile;
#define tfile tfile["txtfile.log"]
#else
#define tfile /##/
#endif
//--------------------------------

TxtFile::TxtFile()
{
}

TxtFile::TxtFile(VFile *pVFile)
{
	Load(pVFile);
}

TxtFile::~TxtFile()
{
}

bool TxtFile::Load(VFile *pVFile)
{
	std::vector<std::string> vString;
	std::string str;
	std::string cell;
	bool bEOS = true;

	//	запоминаем им€ файла
	if(pVFile->Name()) m_FileName = pVFile->Name();
	//	освобождаем контейнер данных
	m_vData.clear();
	vString.clear();
	//	провер€ем на существование файла
	if(pVFile->Size() != 0)
	{//	файл не пустой
		tfile("file: [%s]\n",pVFile->Name());
		pVFile->Rewind();
		while(GetCell(pVFile,cell,&bEOS))
		{
			tfile("[%s]\n",cell.c_str());
			vString.push_back(cell);
			if(bEOS)
			{
				m_vData.push_back(vString);
				tfile("-----------------------------\n");
				vString.clear();
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool TxtFile::GetCell(VFile *pVFile,std::string& str,bool *pbEOS)
{
	bool bComplex = false;

	//	очищаем строку
	str.clear();
	if((pVFile->Tell()!=pVFile->Size()))
	{
		*pbEOS = false;
		if(pVFile->Data()[pVFile->Tell()] == '\"')
		{//	€чейка в ковычках, заканчиваетс€ "\r\n, но не ""\r\n
			//	пропускаем ее
			pVFile->ReadChar();
			bComplex = true;
		}
		if((pVFile->Tell()!=pVFile->Size()))
		{
			while((pVFile->Tell()!=pVFile->Size()))
			{
				//	возможно "" или конец строки
				if(pVFile->Data()[pVFile->Tell()] == '\"')
				{
					//	пропускаем ее
					pVFile->ReadChar();
					if((pVFile->Tell()!=pVFile->Size()))
					{
						if(pVFile->Data()[pVFile->Tell()] != '\"')
						{
							if((pVFile->Data()[pVFile->Tell()] == '\n') ||
							   (pVFile->Data()[pVFile->Tell()] == '\r'))
							{
								bComplex = false;
							}
							continue;
						}
					}
				}
				//	конец строки
				if(pVFile->Data()[pVFile->Tell()] == '\n')
				{
					if(!bComplex)
					{
						*pbEOS = true;
						break;
					}
				}
				//	знак табул€ции - значит следующа€ есть €чейка
				if(pVFile->Data()[pVFile->Tell()] == '\t')
				{
					break;
				}
				//	пропускаем знак \r
				if(pVFile->Data()[pVFile->Tell()] == '\r')
				{
					pVFile->ReadChar();
					continue;
				}
				str.push_back(pVFile->ReadChar());
			}
		}
		//	переводим указатель на следующий символ
		pVFile->ReadChar();
		return true;
	}

	return false;
}

void TxtFile::GetCell(unsigned int i,unsigned int j,std::string& cell)
{
	if(i>=m_vData.size())
		throw CASUS(m_FileName+std::string(": выход за границу массива строк. \n"));
	if(j>=m_vData[i].size())
		throw CASUS(m_FileName+std::string(": выход за границу массива. \n"));
	cell = m_vData[i][j];
}

const std::string& TxtFile::operator()(unsigned int i,unsigned int j)
{
	if(i>=m_vData.size())
		throw CASUS(m_FileName+std::string(": выход за границу массива строк. \n"));
	if(j>=m_vData[i].size())
		throw CASUS(m_FileName+std::string(": выход за границу массива. \n"));
	return m_vData[i][j];
}

//	выдает строку из второй €чейки, перва€ €чейка которой содержит pFind
bool TxtFile::Find(const char *pFind,std::string& cell)
{
	if(pFind)
	{
		for(unsigned int i=0;i<m_vData.size();i++)
		{
			if(m_vData[i].size()>1)
			{
				if(strcmp(m_vData[i][0].c_str(),pFind)==0)
				{
					cell = m_vData[i][1];
					return true;
				}
			}
		}
	}

	return false;
}
//	выдает координаты первой €чейки в столбце j, в которой найдена входна€ строка pFind
bool TxtFile::FindInCol(const char *pFind,unsigned int *i,unsigned int j)
{
	if(pFind)
	{
		for(unsigned int ic=0;ic<m_vData.size();ic++)
		{
			if(strcmp(m_vData[ic][j].c_str(),pFind)==0)
			{
				*i = ic;
				return true;
			}
		}
	}

	return false;
}
//	выдает координаты первой €чейки в строке i, в которой найдена входна€ строка pFind
bool TxtFile::FindInRow(const char *pFind,unsigned int i,unsigned int *j)
{
	if(pFind)
	{
		for(unsigned int jc=0;jc<m_vData[i].size();jc++)
		{
			if(strcmp(m_vData[i][jc].c_str(),pFind)==0)
			{
				*j = jc;
				return true;
			}
		}
	}

	return false;
}

