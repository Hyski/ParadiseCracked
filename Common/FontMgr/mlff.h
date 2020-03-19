/**************************************************************************

							 MiST land Fast Font

                          Copyright by MiST land 2000
  -------------------------------------------------------------------------
  Description: 
  -------------------------------------------------------------------------
  Структура файла:
  0x00000000				MLFF_FILEHEADER
							lSize = sizeof(MLF_FILEHEADER)	- используется 
							для	контроля версий.
							sID = "MiST land Fast Font\0" - используется для 
							идентификации.
							Version - версия шрифта 
										Version[0] - мажор (1)
										Version[1] - минор (0)
  +sizeof(MLFF_FILEHEADER)   MLF_DATAHEADER
							lSize = sizeof(MLF_DATAHEADER)	- используется 
							для	контроля версий.
							lFontHeight - высота строчки шрифта.
                            rcChar[96] - массив, где хранятся координаты 
							для каждой буквы шрифта.
  +sizeof(MLFF_DATAHEADER)   битовый образ шрифта в формате 8 бит на пиксел.
  -------------------------------------------------------------------------
  Version: 1.00
  Coder: Naughty
  Date:  09.10.2000

***************************************************************************/
#ifndef _MLFF_H_
#define _MLFF_H_

struct MLFF_RECT
{
	int left,top,right,bottom;
};

struct MLFF_ABC
{
	int a;
	unsigned int b;
	int c;
};

struct MLFF_FILEHEADER
{
	long lSize;					//	размер заголовка - контроль версий
	char sID[20];				//	идентификатор == "MiST land Fast Font\0"
	unsigned char Version[2];	//	данная версия шрифта
};

struct MLFF_DATAHEADER
{
	long lSize;					//	размер заголовка - контроль версий
	long lFontHeight;			//	высота шрифта
	MLFF_RECT rcChar[96];		//	координаты каждой буквы
	MLFF_ABC abcChar[96];		//	abc для буковок
	long lWidth;				//	ширина битового образа в пикселях
	long lHeight;				//	высота битового образа в пикселях
};

#endif	//_MLFF_H_