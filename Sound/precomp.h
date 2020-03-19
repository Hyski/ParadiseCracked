/***********************************************************************

                          DX8Sound component

                       Copyright by MiST land 2001

   --------------------------------------------------------------------
    Description: DirectSound8 component
   --------------------------------------------------------------------
    Purpose: Manages any sound and music activities
   --------------------------------------------------------------------
    Author: Sergei V. Zagursky (GvozdodeR)

************************************************************************/
#if !defined(__DX8SOUND_PRECOMP_H_INCLUDED__)
#define __DX8SOUND_PRECOMP_H_INCLUDED__

#if defined(_USE_CARCASS_)
#include <common/plug.h>
#include <common/id.h>
#include <common/Error.h>
#endif // _USE_CARCASS_

#define _API extern "C" __declspec(dllexport)

#if defined(_USE_CARCASS_)
// Интерфейсы
#include <interfaces/ISystem.h>
#include <interfaces/ICarcass.h>
#include <interfaces/ICommonInterface.h>
#include <interfaces/ISound.h>
#else
#include "ISound.h"
#endif // _USE_CARCASS_

#include "EnvWrapper.h"
#include "VFS.h"

#include <ole2.h>

#if defined(_DEBUG)
#define TURN_ON_ALL_DEBUG_STUFF
#endif

#if defined(TURN_ON_ALL_DEBUG_STUFF)
#define CORE_USES_LOG
#define CORE_LOG			cc_EnvWrapper::log
#define CORE_WARN_LOG		cc_EnvWrapper::log
#define EMIT_USES_LOG
#define	EMIT_LOG			cc_EnvWrapper::log
#define	EMIT_WARN_LOG		cc_EnvWrapper::log
#else
#define CORE_LOG			/##/
#define EMIT_LOG			/##/
#define CORE_WARN_LOG		/##/
#define EMIT_WARN_LOG		/##/
#endif

#include "DM8Error.h"

#if defined(_UNDER_CARCASS_)
#define COMPTR_H	<common/COMPtr.h>
#define MODDESC_H	<common/ModuleDescription.h>
#else
#define COMPTR_H	"COMPtr.h"
#define MODDESC_H	"ModuleDescription.h"
#endif

#if !defined(STR_FULL_STAMP)
// При отсутствии имени функции вылезет следующее имя
static const char *__strPFuncName = "cc_UnknownClass::unknownFunc()";
// Макросы для выдирания имени функции
#define STR_FULL_STAMP				(std::string("#### ")+\
									 __strPFuncName+" - ")
#define FULL_STAMP					((std::string("#### ")+\
									 __strPFuncName+" - ").c_str())
#endif

#define STR_TIME_STAMP	cc_DM8Error::getMusicTime()

#endif