/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
	Date:   01.08.2000

************************************************************************/
#include "Precomp.h"
#include "decs.h"
#include "ZipInfo.h"

//---------- Ëמד פאיכ ------------
#ifdef __DEBUG_SHELL
#include "..\Log\Log.h"
CLog unzip;
#define zip unzip["zip.log"]
#else
#define zip /##/
#endif
//--------------------------------

namespace ZipInfo
{
	int  WINAPI Password(char *p, int n, const char *m, const char *name);
	void WINAPI ReceiveDllMessage(unsigned long ucsize, unsigned long csiz,
								  unsigned cfactor, unsigned mo, unsigned dy, 
								  unsigned yr, unsigned hh, unsigned mm,
								  char c, LPSTR filename, LPSTR methbuf, 
								  unsigned long crc, char fCrypt);
	int WINAPI DisplayBuf(LPSTR buf, unsigned long size);
	int WINAPI GetReplaceDlgRetVal(char *filename);
	void FreeUzpBuffer(void);
}

namespace ZipInfo
{
	USERFUNCTIONS UserFunctions = 
	{
		DisplayBuf,/*DLLPRNT *print;*/
		0,//DLLSND *sound;
		GetReplaceDlgRetVal,//DLLREPLACE *replace;
		Password,//DLLPASSWORD *password;
		ReceiveDllMessage,//DLLMESSAGE *SendApplicationMessage;
		0,//DLLSERVICE *ServCallBk;
		0,//unsigned long TotalSizeComp;
		0,//unsigned long TotalSize;
		0,//int CompFactor;
		0,//unsigned int NumMembers;
		0,//WORD cchComment;
	};
	UzpBuffer m_UzpBuffer = {0,0};
	//-------------------------------------------------------
	std::vector<std::string> m_vListing;

}

void ZipInfo::ListZip(const char *szZipName,std::vector<std::string>& vStr)
{
	DCL dcl;
	
	memset(&dcl,0,sizeof(DCL));

	dcl.nvflag = 1;
	dcl.lpszZipFN = (char *)szZipName; 

	zip("\nListing for <%s>:\n\n",szZipName);

	Wiz_NoPrinting(true);
	Wiz_SingleEntryUnzip(0,NULL,0,NULL,&dcl,&UserFunctions);
	vStr = m_vListing;
	m_vListing.clear();
}

bool ZipInfo::LoadFile(const char *szZipName,
					   const char *szFileName,
					   unsigned char **pData,
					   unsigned int *pSize)
{
	*pData = 0;
	*pSize = 0;

	Wiz_NoPrinting(true);
	zip("Load <%s> from zip <%s>: ",szFileName,szZipName);
    if(Wiz_UnzipToMemory((char *)szZipName,(char *)szFileName,&UserFunctions,&m_UzpBuffer)) 
	{
		zip("true\n");
		if(m_UzpBuffer.strlength != 0)
		{
			if(m_UzpBuffer.strptr)
			{
				*pSize = m_UzpBuffer.strlength;
				*pData = new unsigned char[*pSize];
				memcpy(*pData,m_UzpBuffer.strptr,*pSize);
			}
		}
		FreeUzpBuffer();

		return true;
    }

	zip("false\n");
	FreeUzpBuffer();

    return false;
}

bool ZipInfo::UnzipFile(const char *szZipName,
						const char *szFileName)
{
	FILE *hFile;
	unsigned char *pData = 0;
	unsigned int iSize;
	bool bOk = false;

	if(LoadFile(szZipName,szFileName,&pData,&iSize))
	{
		if((hFile = fopen(szFileName,"wb")))
		{
			if(fwrite(pData,iSize,1,hFile) == 1)
			{
				bOk = true;
			}
			fclose(hFile);
		}
		if(pData) 
			delete[] pData;
	}

	return bOk;
}

//*****************************************************************************//
int WINAPI ZipInfo::Password(char *p, int , const char *, const char *)
{
 p[ 0]='s';
 p[ 1]='t';
 p[ 2]='e';
 p[ 3]='r';
 p[ 4]='e';
 p[ 5]='o';
 p[ 6]='s';
 p[ 7]='y';
 p[ 8]='s';
 p[ 9]='t';
 p[10]='e';
 p[11]='m';
 p[12]=0;
	return IZ_PW_ENTERED;
}

void WINAPI ZipInfo::ReceiveDllMessage(unsigned long , unsigned long ,
									   unsigned , unsigned , unsigned , 
									   unsigned , unsigned , unsigned ,
									   char , LPSTR filename, LPSTR , 
									   unsigned long , char )
{
	m_vListing.push_back(filename);
	zip("%s\n",filename);
}

int WINAPI ZipInfo::DisplayBuf(LPSTR buf, unsigned long size)
{
	zip("%s",(char *)buf);
	return (unsigned int) size;
}

int WINAPI ZipInfo::GetReplaceDlgRetVal(char *)
{
	return 1;
}

void ZipInfo::FreeUzpBuffer(void)
{
	if(m_UzpBuffer.strlength != 0)
	{
		if(m_UzpBuffer.strptr)
		{
			free(m_UzpBuffer.strptr);
			m_UzpBuffer.strptr = 0;
			m_UzpBuffer.strlength = 0;
		}
	}
}

