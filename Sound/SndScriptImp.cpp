#include "precomp.h"

#include "pcctscfg.h"
#include "pccts_stdio.h"
#include "tokens.h"
#include "cc_SndScriptParser.h"

#include "DX8Sound.h"
#include "Script.h"
#include "ScriptMgr.h"
#include "Precached.h"
#include "Discarded.h"

#include "DM8Core.h"

void cc_SndScriptParser::setDefaults(ANTLRTokenType type)
{
	m_ScriptParams.m_Wave = "nosound.wav";
	m_ScriptParams.m_Volume = 1.0f;
	m_ScriptParams.m_Pan = 0.0f;
	m_ScriptParams.m_ConeIn = 360.0f;
	m_ScriptParams.m_ConeOut = 360.0f;
	m_ScriptParams.m_ConeOutVol = 1.0f;
	m_ScriptParams.m_MinDist = 5.0f;
	m_ScriptParams.m_MaxDist = 100.0f;
	m_UnitName = "noname";
	m_ScriptParams.m_Permanent = false;

	switch (type)
	{
		case KWD_THEME:
			m_ScriptParams.m_Type = ci_Sound::cThemes;
			m_ScriptParams.m_Disable3D = true;
			m_ScriptParams.m_Repeat = 0xFFFFFFFF;
			break;
		case KWD_SPEECH:
			m_ScriptParams.m_Type = ci_Sound::cSpeech;
			m_ScriptParams.m_Disable3D = true;
			m_ScriptParams.m_Repeat = 0;
			break;
		case KWD_EFFECT:
			m_ScriptParams.m_Type = ci_Sound::cEffects;
			m_ScriptParams.m_Disable3D = false;
			m_ScriptParams.m_Repeat = 0;
			break;
		case KWD_MENU:
			m_ScriptParams.m_Type = ci_Sound::cMenu;
			m_ScriptParams.m_Disable3D = false;
			m_ScriptParams.m_Repeat = 0;
			break;
		case KWD_AMBIENT:
			m_ScriptParams.m_Type = ci_Sound::cAmbient;
			m_ScriptParams.m_Disable3D = false;
			m_ScriptParams.m_Repeat = 0xFFFFFFFF;
			break;
		case KWD_DEBUG:
			m_ScriptParams.m_Type = ci_Sound::cDebug;
			m_ScriptParams.m_Disable3D = false;
			m_ScriptParams.m_Repeat = 0;
			break;
	}
}

void cc_SndScriptParser::setName(const char *name)
{
	m_UnitName = name;
}

void cc_SndScriptParser::setFile(const char *name)
{
	char *buffer = new char[strlen(name)];
	strcpy(buffer,name+1);
	buffer[strlen(buffer)-1] = '\0';

	m_ScriptParams.m_Wave = buffer;

	delete [] buffer;
}

void cc_SndScriptParser::setRepeat(unsigned times)
{
	m_ScriptParams.m_Repeat = times;
}

void cc_SndScriptParser::set3D(bool use)
{
	m_ScriptParams.m_Disable3D = !use;
}

void cc_SndScriptParser::setPan(float pan)
{
	m_ScriptParams.m_Pan = pan;
}

void cc_SndScriptParser::setDistances(float minDist, float maxDist)
{
	m_ScriptParams.m_MinDist = minDist;
	m_ScriptParams.m_MaxDist = maxDist;
}

void cc_SndScriptParser::setCone(float minAngle, float maxAngle, float outVol)
{
	m_ScriptParams.m_ConeIn = minAngle;
	m_ScriptParams.m_ConeOut = maxAngle;
	m_ScriptParams.m_ConeOutVol = outVol;
}

void cc_SndScriptParser::setVolume(float vol)
{
	m_ScriptParams.m_Volume = vol;
}

void cc_SndScriptParser::setPermanent()
{
	m_ScriptParams.m_Permanent = true;
}

void cc_SndScriptParser::setManagers(cc_ScriptMgr *scriptMgr, cc_SegmentMgr *segmentMgr)
{
	m_scriptMgr = scriptMgr;
	m_segmentMgr = segmentMgr;
}

void cc_SndScriptParser::pushScript()
{
	cc_SndScript *script = new cc_SndScript(&m_ScriptParams,m_UnitName.c_str(),m_segmentMgr);
	m_scriptMgr->addScript(script);
}

void cc_SndScriptParser::setCacheType(ANTLRTokenType type)
{
	m_CacheType = type;
}

void cc_SndScriptParser::addFileMask(const char *name)
{
	char *buffer = new char[strlen(name)];
	strcpy(buffer,name+1);
	buffer[strlen(buffer)-1] = '\0';

	m_FileMasks.push_back(buffer);
	
	delete [] buffer;
}

void cc_SndScriptParser::addExceptMask(const char *name)
{
	char *buffer = new char[strlen(name)];
	strcpy(buffer,name+1);
	buffer[strlen(buffer)-1] = '\0';

	m_ExceptMasks.push_back(buffer);
	
	delete [] buffer;
}

void cc_SndScriptParser::pushCache()
{
/*	cc_SndCache *cache;
	switch (m_CacheType)
	{
		case KWD_PRECACHE:
			cache = new cc_SndPrecachedCache(this->m_);
			break;
		case KWD_DISCARD:
			cache = new cc_SndDiscardedCache(this);
			break;
		default:
			return;
	}

	{
		for (std::list<std::string>::iterator i = m_FileMasks.begin(); i != m_FileMasks.end(); i++)
			cache->files(i->c_str());
	}

	{
		for (std::list<std::string>::iterator i = m_ExceptMasks.begin(); i != m_ExceptMasks.end(); i++)
			cache->except(i->c_str());
	}

	cc_DX8Sound::m_Sound->getCacheMgr()->addCache(m_UnitName.c_str(),cache);*/
}

void cc_SndScriptParser::clean()
{
	m_ExceptMasks.clear();
	m_FileMasks.clear();
}