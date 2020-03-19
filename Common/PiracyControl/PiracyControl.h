/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2002

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   11.07.2002

************************************************************************/
#ifndef _PIRACY_CONTROL_H_
#define _PIRACY_CONTROL_H_

//=====================================================================================//
//                                struct PiracyControl                                 //
//=====================================================================================//
struct PiracyControl
{
	//	проверка наличия мультиков и их размера
	static bool checkFilmsSize(const std::string& path);

private:

	//	структура, содержащая информацию о файлах мультиков
	struct MultInfo
	{
		unsigned int m_quantity;		//	кол-во мультиков
		unsigned int m_size;			//	общий размер мультиков
	};

	//	получение списка мультов и их размеров
	static const MultInfo& getMultInfo(const std::string& path);

};

//	проверка наличия мультиков и их размера
//	путь имеет форму: "c:\\path\\"
__forceinline bool PiracyControl::checkFilmsSize(const std::string& path)
{
	const MultInfo& mi = getMultInfo(path);
#if !defined(DEMO_VERSION)
	if(mi.m_quantity < 8) return false;
	if(mi.m_size < 120000000) return false;
#endif

	return true;
}



#endif	//_PIRACY_CONTROL_H_