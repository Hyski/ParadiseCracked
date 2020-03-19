#if !defined(__EXECUTION_ENVIRONMENT_WRAPPER_INCLUDED__)
#define __EXECUTION_ENVIRONMENT_WRAPPER_INCLUDED__

#if !defined(_UNDER_CARCASS_)
#include <stdio.h>
#include <map>
#include <string>
#include "../Common/Log/Log.h"
#endif

#if defined(_UNDER_CARCASS_)
#define SND_EXCEPTION cc_Unexpected
#else
#include "../Common/Exception/Exception.h"
#define SND_EXCEPTION CASUS
#endif


// ������� ��� ������ ���������� �������� ����������
class cc_EnvWrapper
{
#if !defined(_UNDER_CARCASS_)
	static CLog m_Log;
#else
	DECLARE_LOG_MEMBER(m_log);
#endif

public:
	enum {LONG_BUFFER_LENGTH = 1025};

	// ��������� ������ � ���-���� fileName
	// �����������: ����� ����������� ������ ���������� ���������� LONG_BUFFER_LENGTH
	static void log (const char *fileName, const char *format, ...);
	static void log (const char *fileName, const std::string &);
};

#endif