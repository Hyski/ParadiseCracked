/**************************************************************************

								MiST land Font

                          Copyright by MiST land 2000
  -------------------------------------------------------------------------
  Description: формат шрифта, используемого MiST land на данный момент при
               создании 3D игр.
  -------------------------------------------------------------------------
  Структура файла:
  0x00000000				MLF_FILEHEADER
							lSize = sizeof(MLF_FILEHEADER)	- используется 
							для	контроля версий.
							sID = "MiST land Font\0" - используется для 
							идентификации.
							Version - версия шрифта 
										Version[0] - мажор (1)
										Version[1] - минор (0)
  +sizeof(MLF_FILEHEADER)   MLF_DATAHEADER
							lSize = sizeof(MLF_DATAHEADER)	- используется 
							для	контроля версий.
							lFontHeight - высота строчки шрифта.
                            rcChar[256] - массив, где хранятся координаты 
							для каждой буквы шрифта.
  +sizeof(MLF_DATAHEADER)   битовый образ шрифта в формате 8 бит на пиксел.
  -------------------------------------------------------------------------
  Version: 1.00
  Coder: Naughty
  Date:  13.06.2000

***************************************************************************/
#ifndef _MLF_H_
#define _MLF_H_

#pragma pack(push,1)

struct MLF_RECT
{
	int left,top,right,bottom;
};

struct MLF_ABC
{
	int a;
	unsigned int b;
	int c;
};

struct MLF_FILEHEADER
{
	long lSize;					//	размер заголовка - контроль версий
	char sID[15];				//	идентификатор == "MiST land Font\0"
	unsigned char Version[2];	//	данная версия шрифта
};

struct MLF_DATAHEADER
{
	long lSize;					//	размер заголовка - контроль версий
	long lFontHeight;			//	высота шрифта
	MLF_RECT rcChar[256];		//	координаты каждой буквы
	MLF_ABC abcChar[256];		//	abc для буковок
	long lWidth;				//	ширина битового образа в пикселях
	long lHeight;				//	высота битового образа в пикселях
};

#pragma pack(pop)

#endif	//_MLF_H_