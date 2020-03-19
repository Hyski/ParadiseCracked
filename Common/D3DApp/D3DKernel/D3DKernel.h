/***********************************************************************

                               Virtuality

                       Copyright by MiST land 2000

   --------------------------------------------------------------------
    Description: модуль является ядром инициализация Direct3D.
                 Основные методы получения информации:
				 - GetDD();         // получение объекта DirectDraw
				 - GetPS();         // получение объекта PrimarySurface
				 - GetBB();         // получение объекта BackBuffer
				 - GetD3D();	    // получение объекта Direct3D
				 - GetD3DDevice();  // получение объекта Direct3DDevice

   --------------------------------------------------------------------
    Author: Pavel A.Duvanov (Naughty)
    Date:   09.05.2000

************************************************************************/
#ifndef _D3D_KERNEL_H_
#define _D3D_KERNEL_H_

namespace D3DKernel
{
	//	инициализация
	BOOL Init(HWND hWnd,
			  GUID* lpDriverGUID,
			  GUID* lpDeviceGUID,
			  int width,
			  int height,
			  int depth,
			  int freq,
			  bool bWindowed);
	//	устанавить использование TripleBuffering'а
	void SetTriple(BOOL bTriple);
	//	освобождаем ресурсы
	void Release(void);
	//	автоматическое переключение страниц
	void UpdateFrame(void);
	//	переключение страниц
	void Flip(void);
	//	блиттинг
	void Blt(void);
	//	востановление поверхностей
	void RestoreSurfaces(void);
	//	перемещение ViewPort'а
	void MoveViewPort(int x,int y);
	//	получение информации
	LPDIRECTDRAW7		 GetDD(void);
	LPDIRECTDRAWSURFACE7 GetPS(void);
	LPDIRECTDRAWSURFACE7 GetBB(void);
	LPDIRECT3D7          GetD3D(void);
	LPDIRECT3DDEVICE7    GetD3DDevice(void);
	int ResX(void);
	int ResY(void);
	BOOL IsFullScreen(void);
}

#endif	//_D3D_KERNEL_H_