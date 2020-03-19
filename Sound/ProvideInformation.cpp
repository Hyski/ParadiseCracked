/***********************************************************************

                          World component

                       Copyright by MiST land 2001

   --------------------------------------------------------------------
    Description:
   --------------------------------------------------------------------
    Author: Sergei V. Zagursky (GvozdodeR)

************************************************************************/
#include "precomp.h"

#if defined(_UNDER_CARCASS_)
#include <common/ModuleDescription.h>

#define MODULE_COUNT	1

cc_ModuleDescription g_desc[MODULE_COUNT] = {{ID_SOUND, "DX8SoundV2", "ConnectDX8Sound"}};

#define _API extern "C" __declspec(dllexport)

_API cc_ModuleDescription *GetDescription (unsigned n)
{
	if (n < MODULE_COUNT) return g_desc + n;
	return 0;
}
#endif