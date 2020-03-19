#include "precomp.h"
#include "DX8Sound.h"
#include <stdarg.h>
#include <stdio.h>

#if !defined(_UNDER_CARCASS_)
CLog cc_EnvWrapper::m_Log;
#else
#include <common/streambuf.h>
LOG_MEMBER_IMPL(cc_EnvWrapper,m_log,"gvz_sound.log");
#define SOUND_LOG		C_HOMELOG(m_log)
#endif

void cc_EnvWrapper::log (const char *fileName, const char *format, ...)
{
	va_list list;
	char longBuffer[LONG_BUFFER_LENGTH];

	va_start(list,format);
	vsprintf(longBuffer, format, list);
	va_end(list);

#if defined(_UNDER_CARCASS_)
	SOUND_LOG << ml_printf("%s",longBuffer);
#else
	m_Log[fileName](longBuffer);
#endif
}

void cc_EnvWrapper::log (const char *fileName, const std::string &str)
{
#if defined(_UNDER_CARCASS_)
	SOUND_LOG << str;
#else
	m_Log[fileName](const_cast<char *>(str.c_str()));
#endif
}
