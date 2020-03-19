/***********************************************************************

                             Virtuality

                       Copyright by MiST land 2000

************************************************************************/
#include "Precomp.h"
#include "Common\Shell\Shell.h"
#include "globals.h"

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
/*
#ifdef _DEBUG
    //ведение статистики памяти
    _CrtMemState mem_state1, mem_state2, mem_state3;
    _CrtMemCheckpoint(&mem_state1);

    //отлов утечек памяти
    _CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)|_CRTDBG_LEAK_CHECK_DF);
#endif
	*/

	WORD wOldCW;
    try
	{
		BOOL bChangedFPCW = MungeFPCW(&wOldCW);

		Shell shell;			
			  shell.Go();

		if(bChangedFPCW) 
			RestoreFPCW(wOldCW);
	}
	catch(CasusImprovisus& ci)
	{
		MessageBox(0,
				   ci.Content(),
				   "Paradise Cracked Exception",
				   MB_OK|MB_ICONERROR);
	}

	/*
#ifdef _DEBUG
    //вывод статистики использования памяти
    _CrtMemCheckpoint(&mem_state2);
    _CrtMemDifference(&mem_state3, &mem_state1, &mem_state2);
    _RPT0( _CRT_WARN, "\n\n~~~~~~~~Memory Statics~~~~~~~~~\n\n");
    _CrtMemDumpStatistics(&mem_state3);
    _RPT0( _CRT_WARN, "\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
#endif
*/
	return 0;
}
