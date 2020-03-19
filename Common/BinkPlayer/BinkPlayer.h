/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2001

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   20.02.2001

************************************************************************/
#ifndef _BINK_PLAYER_H_
#define _BINK_PLAYER_H_

#include "../Bink/Bink.h"

class BinkPlayer
{
private:
	struct Deleter 
	{
	public:
		BinkPlayer *m_pInstance;
	public:
		Deleter(){m_pInstance = 0;}
		~Deleter(){if(m_pInstance) delete m_pInstance; m_pInstance = 0;}
	};
	friend Deleter;
	static Deleter m_Deleter;
private:
	HBINK m_hBink;
private:
	BinkPlayer();
	virtual ~BinkPlayer();
public:
	//	********************** ИНТЕРФЕЙС *************************** //
	bool Open(const char *pFileName);	//	открыть бик файл
	void Close(void);					//	закрыть бик файл
	void NextFrame(void);				//	распаковать следующий фрейм
	bool Wait(void);					//	синхронизация
	void Render(LPDIRECTDRAWSURFACE7 pBackSurface);
	//	***************** ИНФОРМАЦИОННЫЕ ФУНКЦИИ ****************** //
	int Width(void);					//	ширина изображения
	int Height(void);					//	высота изображения
	bool IsFinish(void);				//	закончил ли бик проигрываться
public:
	static BinkPlayer *Instance(void);
};

inline BinkPlayer *BinkPlayer::Instance(void)
{
	if(!m_Deleter.m_pInstance) m_Deleter.m_pInstance = new BinkPlayer();
	return m_Deleter.m_pInstance;
}
//	ширина изображения
inline int BinkPlayer::Width(void)
{
	return m_hBink->Width;
}
//	высота изображения
inline int BinkPlayer::Height(void)
{
	return m_hBink->Height;
}
//	закончил ли бик проигрываться
inline bool BinkPlayer::IsFinish(void)
{
	return m_hBink?(m_hBink->FrameNum==(m_hBink->Frames-1)):true;
}

#endif	//_BINK_PLAYER_H_