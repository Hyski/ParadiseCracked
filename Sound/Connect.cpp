#include "precomp.h"
#include "DX8Sound.h"

_API ci_CommonInterface *ConnectDX8Sound ()
{
	cc_DX8Sound::m_Sound = new cc_DX8Sound;
	return cc_DX8Sound::m_Sound;
}
