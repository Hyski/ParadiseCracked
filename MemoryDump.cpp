#include "precomp.h"

#ifdef _DEBUG
class MemoryDump
{
public:
	MemoryDump();
	~MemoryDump();
private:
	_CrtMemState mem_state1;
	_CrtMemState mem_state2;
	_CrtMemState mem_state3;
};

MemoryDump::MemoryDump()
{
    //ведение статистики памяти
    _CrtMemCheckpoint(&mem_state1);

    //отлов утечек памяти
    _CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)|_CRTDBG_LEAK_CHECK_DF);
}

MemoryDump::~MemoryDump()
{
    //вывод статистики использования памяти
    _CrtMemCheckpoint(&mem_state2);
    _CrtMemDifference(&mem_state3, &mem_state1, &mem_state2);
    _RPT0( _CRT_WARN, "\n\n~~~~~~~~Memory Statics~~~~~~~~~\n\n");
    _CrtMemDumpStatistics(&mem_state3);
    _RPT0( _CRT_WARN, "\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
}

#pragma init_seg(lib)
MemoryDump memory_dump;
#endif
